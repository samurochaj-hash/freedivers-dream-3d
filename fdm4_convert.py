#!/usr/bin/env python3
"""Convert a structured JSON interchange file into FDM4 v1 binary data."""
import argparse
import json
import math
import struct
import sys
from pathlib import Path

MAX_VERTICES = 8192
MAX_INDICES = 24576
MAX_JOINTS = 32
MAX_CLIPS = 16
MAX_KEYS = 64
NAME_BYTES = 24

def fail(message):
    raise ValueError(message)

def number(value, label):
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        fail(f"{label}: expected a number")
    value = float(value)
    if not math.isfinite(value):
        fail(f"{label}: value must be finite")
    return value

def vector(value, length, label):
    if not isinstance(value, list) or len(value) != length:
        fail(f"{label}: expected an array with {length} numbers")
    return [number(x, f"{label}[{i}]") for i, x in enumerate(value)]

def transform(obj, label):
    if not isinstance(obj, dict):
        fail(f"{label}: expected an object")
    return (
        vector(obj.get("position", [0, 0, 0]), 3, label+".position") +
        vector(obj.get("rotation", [0, 0, 0, 1]), 4, label+".rotation") +
        vector(obj.get("scale", [1, 1, 1]), 3, label+".scale")
    )

def pack_u16(v):
    if not 0 <= int(v) <= 65535:
        fail("u16 value out of range")
    return struct.pack("<H", int(v))

def pack_i16(v):
    if not -32768 <= int(v) <= 32767:
        fail("i16 value out of range")
    return struct.pack("<h", int(v))

def pack_u32(v):
    if not 0 <= int(v) <= 0xffffffff:
        fail("u32 value out of range")
    return struct.pack("<I", int(v))

def pack_floats(values):
    return struct.pack("<" + "f"*len(values), *values)

def convert(document):
    if not isinstance(document, dict):
        fail("top-level JSON must be an object")
    vertices = document.get("vertices", [])
    indices = document.get("indices", [])
    joints = document.get("joints", [])
    clips = document.get("clips", [])
    if not isinstance(vertices, list) or not vertices:
        fail("vertices must be a non-empty array")
    if not isinstance(indices, list):
        fail("indices must be an array")
    if not isinstance(joints, list) or not joints:
        fail("joints must be a non-empty array")
    if not isinstance(clips, list):
        fail("clips must be an array")
    if len(vertices) > MAX_VERTICES: fail(f"too many vertices (max {MAX_VERTICES})")
    if len(indices) > MAX_INDICES: fail(f"too many indices (max {MAX_INDICES})")
    if len(joints) > MAX_JOINTS: fail(f"too many joints (max {MAX_JOINTS})")
    if len(clips) > MAX_CLIPS: fail(f"too many clips (max {MAX_CLIPS})")
    if len(indices) % 3: fail("indices must describe complete triangles (multiple of 3)")

    out = bytearray()
    all_tracks = []
    clip_meta = []

    # Header is written after counts are known.
    vertex_blob = bytearray()
    for vi, vertex in enumerate(vertices):
        if not isinstance(vertex, dict): fail(f"vertices[{vi}] must be an object")
        pos = vector(vertex.get("position"), 3, f"vertices[{vi}].position")
        normal = vector(vertex.get("normal", [0, 1, 0]), 3, f"vertices[{vi}].normal")
        influences = vertex.get("weights", [])
        if not isinstance(influences, list) or len(influences) > 4:
            fail(f"vertices[{vi}].weights must contain 0 to 4 [joint, weight] pairs")
        pairs = []
        total = 0.0
        for wi, pair in enumerate(influences):
            if not isinstance(pair, list) or len(pair) != 2:
                fail(f"vertices[{vi}].weights[{wi}] must be [joint, weight]")
            joint = pair[0]
            if isinstance(joint, bool) or not isinstance(joint, int) or not 0 <= joint < len(joints):
                fail(f"vertices[{vi}].weights[{wi}] has invalid joint index")
            weight = number(pair[1], f"vertices[{vi}].weights[{wi}].weight")
            if weight < 0: fail("weights cannot be negative")
            pairs.append((joint, weight)); total += weight
        vertex_blob += pack_floats(pos + normal)
        for k in range(4):
            if k < len(pairs):
                ji, wt = pairs[k]
                wt = wt / total if total > 0 else 0.0
            else:
                ji, wt = 0, 0.0
            vertex_blob += pack_u16(ji) + pack_floats([wt])

    index_blob = bytearray()
    for ii, idx in enumerate(indices):
        if isinstance(idx, bool) or not isinstance(idx, int) or not 0 <= idx < len(vertices):
            fail(f"indices[{ii}] is outside the vertex array")
        index_blob += pack_u32(idx)

    joint_blob = bytearray()
    for ji, joint in enumerate(joints):
        if not isinstance(joint, dict): fail(f"joints[{ji}] must be an object")
        parent = joint.get("parent", -1)
        if isinstance(parent, bool) or not isinstance(parent, int) or parent < -1 or parent >= ji:
            fail(f"joints[{ji}].parent must be -1 or an earlier joint index")
        flags = joint.get("flags", 0)
        bind = transform(joint.get("bind_local", {}), f"joints[{ji}].bind_local")
        inv_bind = vector(joint.get("inverse_bind"), 16, f"joints[{ji}].inverse_bind")
        joint_blob += pack_i16(parent) + pack_u16(flags)
        joint_blob += pack_floats(bind) + pack_floats(inv_bind)

    track_blob = bytearray()
    key_blob = bytearray()
    for ci, clip in enumerate(clips):
        if not isinstance(clip, dict): fail(f"clips[{ci}] must be an object")
        duration = number(clip.get("duration"), f"clips[{ci}].duration")
        if duration <= 0: fail(f"clips[{ci}].duration must be positive")
        clip_tracks = clip.get("tracks", [])
        if not isinstance(clip_tracks, list): fail(f"clips[{ci}].tracks must be an array")
        first_track = len(all_tracks)
        for ti, track in enumerate(clip_tracks):
            if not isinstance(track, dict): fail(f"clips[{ci}].tracks[{ti}] must be an object")
            joint_index = track.get("joint")
            if isinstance(joint_index, bool) or not isinstance(joint_index, int) or not 0 <= joint_index < len(joints):
                fail(f"clips[{ci}].tracks[{ti}] has invalid joint")
            keys = track.get("keys", [])
            if not isinstance(keys, list) or not 1 <= len(keys) <= MAX_KEYS:
                fail(f"clips[{ci}].tracks[{ti}] must have 1 to {MAX_KEYS} keys")
            parsed = []
            previous = -1.0
            for ki, key in enumerate(keys):
                if not isinstance(key, dict): fail("keyframe must be an object")
                tm = number(key.get("time"), "keyframe.time")
                if tm < 0 or tm <= previous or tm > duration:
                    fail("keyframe times must increase and stay within clip duration")
                previous = tm
                parsed.append((tm, transform(key.get("transform", {}), "keyframe.transform")))
            all_tracks.append((joint_index, parsed))
        track_count = len(all_tracks) - first_track
        clip_meta.append((clip.get("name", f"clip_{ci}"), duration,
                          int(clip.get("joint_count", len(joints))),
                          track_count, first_track, bool(clip.get("looping", True))))

    if len(all_tracks) > MAX_CLIPS * MAX_JOINTS:
        fail("too many animation tracks")
    total_keys = sum(len(keys) for _, keys in all_tracks)
    if total_keys > MAX_CLIPS * MAX_JOINTS * MAX_KEYS:
        fail("too many keyframes")

    for joint_index, keys in all_tracks:
        track_blob += pack_u16(joint_index) + pack_u16(len(keys))
    for _, keys in all_tracks:
        for tm, tr in keys:
            key_blob += pack_floats([tm]) + pack_floats(tr)

    clip_blob = bytearray()
    for name, duration, joint_count, track_count, first_track, looping in clip_meta:
        if not isinstance(name, str): fail("clip name must be text")
        encoded = name.encode("utf-8")
        if len(encoded) > NAME_BYTES: fail(f"clip name must be at most {NAME_BYTES} UTF-8 bytes")
        if not 0 <= joint_count <= len(joints): fail("clip joint_count out of range")
        clip_blob += encoded.ljust(NAME_BYTES, b"\0")
        clip_blob += pack_floats([duration])
        clip_blob += pack_u16(joint_count) + pack_u16(track_count) + pack_u16(first_track)
        clip_blob += bytes([1 if looping else 0, 0])

    header = (
        b"FDM4" + pack_u16(1) + pack_u16(0) +
        pack_u32(len(vertices)) + pack_u32(len(indices)) +
        pack_u16(len(joints)) + pack_u16(len(clips)) +
        pack_u16(len(all_tracks)) + pack_u16(0) + pack_u32(total_keys)
    )
    return header + vertex_blob + index_blob + joint_blob + track_blob + key_blob + clip_blob

def main():
    parser = argparse.ArgumentParser(description="Convert JSON interchange data to FDM4 v1")
    parser.add_argument("input", help="input JSON file")
    parser.add_argument("output", help="output .fdm4 file")
    args = parser.parse_args()
    try:
        doc = json.loads(Path(args.input).read_text(encoding="utf-8"))
        binary = convert(doc)
        Path(args.output).write_bytes(binary)
        print(f"FDM4 written: {args.output} ({len(binary)} bytes)")
    except (OSError, json.JSONDecodeError, ValueError, struct.error) as exc:
        print(f"FDM4 conversion failed: {exc}", file=sys.stderr)
        return 1
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
