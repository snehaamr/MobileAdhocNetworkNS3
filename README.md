# Mobile Ad-Hoc Network with Buffer Size and Packet Loss Simulation
Overview
This simulation models a Mobile Ad-Hoc Network (MANET) where nodes communicate with each other using a tree-like structure. Each node has a buffer that holds outgoing packets before they are transmitted, and the simulation tracks the number of packets sent, successfully delivered, and dropped due to buffer overflow. The goal is to analyze how varying buffer sizes affect packet transfer, delivery success, and data loss in the network.

Key Features:
Buffer Size Management: Each node has a configurable buffer to hold outgoing packets. If the buffer is full, packets are dropped.
Packet Loss Calculation: The simulation tracks the number of dropped packets due to buffer overflow, as well as the number of successfully delivered packets.
Dynamic Node Mobility: Nodes move randomly in a 2D space, and the simulation reflects changes in their position.
Simulation Metrics: The number of packets sent, delivered, and dropped is periodically printed for each node during the simulation.

Simulation Flow:
Node Creation:

A specified number of nodes (nWifi) are created and configured with mobility models that simulate random movement within a 2D area.
Each node is initialized with a socket for communication.
Application Setup:

Each node runs a TreeStructureApp application that simulates packet transmission and reception. The application uses a buffer to hold packets before they are sent to other nodes.
The buffer has a maximum size (bufferSize), and if the buffer is full, new packets are dropped.
Periodically, nodes attempt to send packets and receive packets from others. Successful deliveries and packet drops due to full buffers are tracked.
Packet Transmission and Reception:

Nodes send packets to a broadcast address. When a node receives a packet, it checks whether the buffer has space for the incoming packet. If there’s space, the packet is added to the buffer and transmitted; otherwise, it is dropped.
Statistics Tracking:

For each node, the following statistics are tracked:
Number of packets sent
Number of packets successfully delivered
Number of packets dropped due to full buffer
These statistics are printed periodically during the simulation.
Packet Loss Analysis:

By varying the buffer size, you can analyze the effect of buffer overflow on packet loss. Nodes with larger buffers can store more packets and reduce the probability of packet loss, while nodes with smaller buffers may experience higher packet loss.

Dependencies:
This project uses the ns-3 simulator, which is an open-source discrete-event network simulator for research and educational use. The following modules are required:

core-module
network-module
applications-module
wifi-module
mobility-module
internet-module

Installation:
Install ns-3: If you haven't installed ns-3 yet, follow the installation instructions from the official ns-3 documentation.

Clone this repository: Clone the repository to your local machine:

git clone https://github.com/your-repo/ns3-buffer-simulation.git
cd ns3-buffer-simulation

Build the Simulation: Once you've cloned the repository, build the simulation using ns-3's build system:
./waf configure
./waf build

Run the Simulation: You can run the simulation with a specified number of nodes and buffer size:
./waf --run "simulation --nWifi=40 --bufferSize=10"

--nWifi=<num>: Number of nodes in the simulation (default: 40).
--bufferSize=<size>: Size of the buffer at each node (default: 10 packets).

uffer Management and Packet Loss Calculation:
Buffer Size: Each node has a buffer that holds a specified number of packets before they are transmitted. The buffer size is configurable.
Packet Loss: If a node's buffer is full, any new packets that it tries to send are dropped, and this packet loss is tracked.
Packet Statistics: For each node, the number of packets sent, successfully delivered, and dropped are tracked and printed periodically during the simulation.

Output Example:
The simulation will print statistics every 10 seconds for each node, showing the number of packets sent, successfully delivered, and dropped due to buffer overflow.

Example output:
NodeNo   Sent    Delivered   Dropped   Time
1        5       5           0         10
2        4       3           1         10
3        3       3           0         10
...

Analyzing Results:
By varying the buffer size and running the simulation, you can analyze:

How increasing the buffer size reduces packet loss.
The trade-off between buffer size and network performance (e.g., memory usage vs. packet delivery rate).

Customization:
You can easily modify the following simulation parameters:

Number of Nodes: Set the number of nodes (nWifi).
Buffer Size: Set the buffer size for each node (bufferSize).
Mobility Parameters: Customize the mobility model to control how nodes move (e.g., speed, range, movement type).
Simulation Time: Adjust the simulation runtime (SetStopTime).


Certainly! Below is an expanded README that includes an explanation of the code, its components, and the changes made for varying buffer sizes and packet transfer statistics.

README: Mobile Ad-Hoc Network with Buffer Size and Packet Loss Simulation
Overview
This simulation models a Mobile Ad-Hoc Network (MANET) where nodes communicate with each other using a tree-like structure. Each node has a buffer that holds outgoing packets before they are transmitted, and the simulation tracks the number of packets sent, successfully delivered, and dropped due to buffer overflow. The goal is to analyze how varying buffer sizes affect packet transfer, delivery success, and data loss in the network.

Key Features:
Buffer Size Management: Each node has a configurable buffer to hold outgoing packets. If the buffer is full, packets are dropped.
Packet Loss Calculation: The simulation tracks the number of dropped packets due to buffer overflow, as well as the number of successfully delivered packets.
Dynamic Node Mobility: Nodes move randomly in a 2D space, and the simulation reflects changes in their position.
Simulation Metrics: The number of packets sent, delivered, and dropped is periodically printed for each node during the simulation.
Simulation Flow:
Node Creation:

A specified number of nodes (nWifi) are created and configured with mobility models that simulate random movement within a 2D area.
Each node is initialized with a socket for communication.
Application Setup:

Each node runs a TreeStructureApp application that simulates packet transmission and reception. The application uses a buffer to hold packets before they are sent to other nodes.
The buffer has a maximum size (bufferSize), and if the buffer is full, new packets are dropped.
Periodically, nodes attempt to send packets and receive packets from others. Successful deliveries and packet drops due to full buffers are tracked.
Packet Transmission and Reception:

Nodes send packets to a broadcast address. When a node receives a packet, it checks whether the buffer has space for the incoming packet. If there’s space, the packet is added to the buffer and transmitted; otherwise, it is dropped.
Statistics Tracking:

For each node, the following statistics are tracked:
Number of packets sent
Number of packets successfully delivered
Number of packets dropped due to full buffer
These statistics are printed periodically during the simulation.
Packet Loss Analysis:

By varying the buffer size, you can analyze the effect of buffer overflow on packet loss. Nodes with larger buffers can store more packets and reduce the probability of packet loss, while nodes with smaller buffers may experience higher packet loss.

This project uses the ns-3 simulator, which is an open-source discrete-event network simulator for research and educational use. The following modules are required:

core-module
network-module
applications-module
wifi-module
mobility-module
internet-module
Installation:
Install ns-3: If you haven't installed ns-3 yet, follow the installation instructions from the official ns-3 documentation.

Clone this repository: Clone the repository to your local machine:


git clone https://github.com/your-repo/ns3-buffer-simulation.git
cd ns3-buffer-simulation
Build the Simulation: Once you've cloned the repository, build the simulation using ns-3's build system:

./waf configure
./waf build
Run the Simulation: You can run the simulation with a specified number of nodes and buffer size:

./waf --run "simulation --nWifi=40 --bufferSize=10"
--nWifi=<num>: Number of nodes in the simulation (default: 40).
--bufferSize=<size>: Size of the buffer at each node (default: 10 packets).
Key Classes and Functions:
1. MobileAdhocTree Class
This class represents the basic structure of each node in the network. It holds the following information:

my_id: Node ID.
parent: Parent node in the tree (for routing).
hopcount: Number of hops from the root node.
x, y: Position of the node in the 2D space.
2. TreeStructureApp Class
This is the main application running on each node. It handles:

Packet Transmission: Nodes attempt to send packets periodically. The packets are enqueued in a buffer before being transmitted.
Packet Reception: Nodes receive packets and manage buffer overflow situations.
Heartbeat: Nodes periodically check and update their hop count and parent information.
Statistics Tracking: Tracks the number of packets sent, delivered, and dropped, and prints these stats periodically.
3. Main Simulation (main.cc)
The main entry point of the simulation where:

Nodes are created and initialized with random mobility.
Applications are installed on each node with a specified buffer size.
Simulation parameters such as the number of nodes and buffer size are set via command-line arguments.
New Features in This Version:
Buffer Management and Packet Loss Calculation:
Buffer Size: Each node has a buffer that holds a specified number of packets before they are transmitted. The buffer size is configurable.
Packet Loss: If a node's buffer is full, any new packets that it tries to send are dropped, and this packet loss is tracked.
Packet Statistics: For each node, the number of packets sent, successfully delivered, and dropped are tracked and printed periodically during the simulation.
Output Example:
The simulation will print statistics every 10 seconds for each node, showing the number of packets sent, successfully delivered, and dropped due to buffer overflow.

Example output:

plaintext
Copy code
NodeNo   Sent    Delivered   Dropped   Time
1        5       5           0         10
2        4       3           1         10
3        3       3           0         10
...
Analyzing Results:
By varying the buffer size and running the simulation, you can analyze:

How increasing the buffer size reduces packet loss.
The trade-off between buffer size and network performance (e.g., memory usage vs. packet delivery rate).
Customization:
You can easily modify the following simulation parameters:

Number of Nodes: Set the number of nodes (nWifi).
Buffer Size: Set the buffer size for each node (bufferSize).
Mobility Parameters: Customize the mobility model to control how nodes move (e.g., speed, range, movement type).
Simulation Time: Adjust the simulation runtime (SetStopTime).
Future Work:
Network Topology: Implement different network topologies such as star, mesh, or grid.
Advanced Routing: Implement more sophisticated routing algorithms to improve packet delivery.
Traffic Generation: Add traffic generation models (e.g., UDP, TCP) to simulate real-world applications more accurately.






