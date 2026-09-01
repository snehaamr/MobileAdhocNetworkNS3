#!/usr/bin/env python3
"""Sweep bufferSize x nWifi and record app-buffer vs FlowMonitor loss.

Run from an ns-3 tree that has this repo at scratch/mobile-adhoc-tree:

  python3 /path/to/MobileAdhocNetworkNS3/scripts/sweep_buffers.py \\
      --ns3 /path/to/ns-3-dev --sim-time 30 --buffers 2,10,20 --nodes 10,20
"""

from __future__ import annotations

import argparse
import csv
import os
import re
import subprocess
import sys
from pathlib import Path

SUMMARY_RE = re.compile(r"^SUMMARY\s+(.*)$", re.MULTILINE)
PAIR_RE = re.compile(r"(\w+)=(\S+)")


def parse_csv_list(raw: str, kind: type):
    values = []
    for part in raw.split(","):
        part = part.strip()
        if not part:
            continue
        values.append(kind(part))
    if not values:
        raise argparse.ArgumentTypeError("expected a comma-separated list")
    return values


def parse_summary(text: str) -> dict[str, str]:
    match = SUMMARY_RE.search(text)
    if not match:
        raise RuntimeError("no SUMMARY line in simulation output")
    return {key: value for key, value in PAIR_RE.findall(match.group(1))}


def run_one(ns3: Path, program_args: str, cwd: Path) -> dict[str, str]:
    cmd = [str(ns3 / "ns3"), "run", "--no-build", program_args]
    env = os.environ.copy()
    proc = subprocess.run(
        cmd,
        cwd=cwd,
        env=env,
        check=False,
        capture_output=True,
        text=True,
    )
    output = (proc.stdout or "") + "\n" + (proc.stderr or "")
    if proc.returncode != 0:
        sys.stderr.write(output)
        raise RuntimeError(f"simulation failed with exit {proc.returncode}: {cmd}")
    return parse_summary(output)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--ns3", type=Path, required=True, help="ns-3 source/build tree (contains ./ns3)")
    parser.add_argument("--buffers", default="1,2,5,10,20", help="comma-separated bufferSize values")
    parser.add_argument("--nodes", default="10,20,40", help="comma-separated nWifi values")
    parser.add_argument("--sim-time", type=float, default=30.0)
    parser.add_argument("--seed", type=int, default=1)
    parser.add_argument("--run", type=int, default=1)
    parser.add_argument("--area-size", type=float, default=200.0)
    parser.add_argument("--outdir", type=Path, default=Path("sweep-out"))
    parser.add_argument("--skip-build", action="store_true")
    args = parser.parse_args()

    ns3 = args.ns3.resolve()
    if not (ns3 / "ns3").exists():
        parser.error(f"no ./ns3 wrapper at {ns3}")

    buffers = parse_csv_list(args.buffers, int)
    nodes = parse_csv_list(args.nodes, int)
    args.outdir.mkdir(parents=True, exist_ok=True)

    if not args.skip_build:
        build = subprocess.run([str(ns3 / "ns3"), "build"], cwd=ns3)
        if build.returncode != 0:
            return build.returncode

    rows = []
    fieldnames = [
        "nWifi",
        "bufferSize",
        "simTime",
        "seed",
        "run",
        "originated",
        "delivered",
        "uniqueDelivered",
        "bufferDropped",
        "attached",
        "pdr",
        "uniquePdr",
        "flowUnicastTx",
        "flowUnicastRx",
        "flowUnicastLost",
        "flowMacQueueDrops",
        "flowNoRouteDrops",
        "csv",
        "flowmon",
    ]

    for n_wifi in nodes:
        for buffer_size in buffers:
            stem = f"n{n_wifi}_b{buffer_size}_s{args.seed}_r{args.run}"
            csv_path = args.outdir / f"{stem}.csv"
            flow_path = args.outdir / f"{stem}.flowmon.xml"
            extra = [
                "mobile-adhoc-tree",
                f"--nWifi={n_wifi}",
                f"--bufferSize={buffer_size}",
                f"--simTime={args.sim_time}",
                f"--seed={args.seed}",
                f"--run={args.run}",
                f"--areaSize={args.area_size}",
                f"--csv={csv_path}",
                f"--flowmon={flow_path}",
            ]
            print(f"Running nWifi={n_wifi} bufferSize={buffer_size} ...", flush=True)
            summary = run_one(ns3, " ".join(extra), ns3)
            row = {name: summary.get(name, "") for name in fieldnames if name not in ("csv", "flowmon")}
            row["csv"] = str(csv_path)
            row["flowmon"] = str(flow_path)
            rows.append(row)

    summary_path = args.outdir / "sweep-summary.csv"
    with summary_path.open("w", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)

    print(f"Wrote {summary_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
