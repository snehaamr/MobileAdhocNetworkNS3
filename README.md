# Mobile Ad-Hoc Network (ns-3) with tree routing and buffer loss

This simulation models a Wi-Fi ad-hoc MANET that organizes itself into a tree rooted at **node 0**. Non-root nodes generate data, store-and-forward through a finite per-node buffer toward their parent, and drop packets when that buffer is full. The goal is to measure how buffer size, density, and mobility affect delivery to the sink.

Requires **ns-3.40 or later** (CMake / `./ns3`). Older `waf` trees and APIs such as `NqosWifiMacHelper` are not supported.

## What the simulation does

1. Creates `nWifi` 802.11b ad-hoc stations in a square area with random-walk mobility.
2. Node 0 is the tree root (sink). Every node broadcasts periodic **beacons** with its id, parent, hop count, and IP address.
3. Other nodes join the neighbor that advertises the smallest hop count (loop-avoiding: they will not choose a child as parent). If a parent’s beacons stop, the node detaches and can rejoin.
4. Non-root nodes **originate** data packets on `sendInterval`. Packets are **enqueued** in a per-node buffer of size `bufferSize`.
5. Independently, each node **drains** at most one buffered packet toward its parent every `txInterval`. If the buffer is full, new originated or forwarded packets are **dropped**.
6. The root counts a packet as **delivered** when it receives data. “Delivered” is not incremented merely because a packet was queued locally.

## Metrics

| Field | Meaning |
| --- | --- |
| Originated | Data packets this node generated |
| Sent | Data packets this node unicast to its parent |
| Delivered | Data packets received **at the root** (0 on non-root nodes) |
| Dropped | Originated or forwarded packets discarded because the buffer was full |
| Unique | Distinct originator+seq packets received at the root |
| PDR | Root deliveries / total originated |
| Unique PDR | Unique root deliveries / total originated |
| FlowMonitor unicast lost | IP-level unicast packets that never arrived (collisions, range, queue) |
| Buffer dropped | Application overflow only — not wireless loss |

Time series are written to CSV (`simTime,nodeId,parent,hopcount,originated,sent,delivered,dropped,bufferOccupancy`). A table, aggregate PDR, FlowMonitor unicast counters, and a `SUMMARY` line are printed when the simulation stops. Optional XML goes to `--flowmon`.

Larger buffers should cut **bufferDropped**. They should not zero out **flowUnicastLost** if the channel is congested or the tree is partitioned.

## Build

Install ns-3 following the [official guide](https://www.nsnam.org/documentation/). Then place this repository inside the ns-3 `scratch` directory (copy or symlink):

```bash
git clone https://github.com/snehaamr/MobileAdhocNetworkNS3.git
cd ns-3-dev   # your ns-3 source tree
ln -s /path/to/MobileAdhocNetworkNS3 scratch/mobile-adhoc-tree
./ns3 configure --enable-examples
# If you use --enable-modules, include flow-monitor (needed for MAC/IP loss stats).
./ns3 build
```

The scratch `CMakeLists.txt` produces an executable named `mobile-adhoc-tree`.

## Run

```bash
./ns3 run "mobile-adhoc-tree --nWifi=40 --bufferSize=10 --simTime=100"
```

Useful arguments:

| Argument | Default | Description |
| --- | --- | --- |
| `--nWifi` | 40 | Number of stations (node 0 is the root) |
| `--bufferSize` | 10 | Packets each node may queue |
| `--simTime` | 100 | Runtime in seconds |
| `--sendInterval` | 2.0 | Data generation period (non-root) |
| `--txInterval` | 0.5 | Buffer drain period |
| `--heartbeatInterval` | 1.0 | Beacon period |
| `--areaSize` | 480 | Square area side length (meters) |
| `--txPower` | 20 | PHY transmit power (dBm) |
| `--seed` / `--run` | 1 / 1 | `RngSeed` / `RngRun` for repeatable trials |
| `--csv` | `manet-stats.csv` | Time-series output path |
| `--flowmon` | `flowmon.xml` | FlowMonitor XML (`""` skips the file) |

Example: compare buffer sizes (same seed):

```bash
./ns3 run "mobile-adhoc-tree --bufferSize=2 --seed=1 --csv=buf2.csv"
./ns3 run "mobile-adhoc-tree --bufferSize=20 --seed=1 --csv=buf20.csv"
```

Or sweep `bufferSize` × `nWifi` (writes `sweep-out/sweep-summary.csv`):

```bash
python3 scratch/mobile-adhoc-tree/scripts/sweep_buffers.py \
  --ns3 . \
  --sim-time 30 \
  --buffers 2,10,20 \
  --nodes 10,20 \
  --outdir sweep-out
```

Larger buffers should reduce overflow drops at busy parents; they do not remove wireless collisions or partitions. Nodes that never hear a beacon stay disconnected (`parent = -1`, hop count printed as `-1`).

## Layout

| File | Role |
| --- | --- |
| `main.cc` | Command-line setup, Wi-Fi/IP, apps, final summary |
| `WiFiNetworkSetup.*` | Ad-hoc Wi-Fi, TX power (set before install), random-walk bounds |
| `TreeStructureApp.*` | Beacons, parent selection, buffer, unicast toward parent |
| `MobileAdhocTree.*` | Packed ns-3 `Header` for beacons and data |
| `CMakeLists.txt` | Scratch executable definition |
| `scripts/sweep_buffers.py` | Parameter sweep → `sweep-summary.csv` |

## Customization

- **Mobility**: speed and walk distance are set in `WiFiNetworkSetup::SetupMobility`.
- **Traffic**: root does not originate data (it is the sink). Change `sendInterval` / `txInterval` so the buffer can fill: generation plus forwarding faster than drain increases overflow.
- **PHY range**: `txPower` and `areaSize` together control how connected the tree is.

## Future work

- Additional topologies (mesh, grid) and routing (AODV/OLSR vs this custom tree).
- NetAnim traces and optional PCAP dumps.
