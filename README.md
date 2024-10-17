
# SAHARA: Secure Ad-Hoc Advanced Resilient Architecture

## Overview

SAHARA (Secure Ad-Hoc Advanced Resilient Architecture) is a collaborative reputation-based system designed to enhance routing decisions and strengthen network security in mobile ad-hoc networks (MANETs) with minimal overhead. This system integrates three key concepts:

- **Routing Protocols**
- **Information Sharing**
- **Network Security**

The system dynamically adapts to changes in the network, providing robust security against common attacks while minimizing the impact on resource-constrained devices, such as those in **robot** and **drone swarms**.

## Features

- **Reputation-based Node Evaluation**: SAHARA leverages reputation mechanisms to assess node trustworthiness, ensuring that only reliable nodes participate in routing decisions.
- **Resilience to Attacks**: The system dynamically responds to malicious behaviors (e.g., packet dropping, DoS attacks), ensuring network throughput recovery in hostile environments.
- **Efficient Information Sharing**: SAHARA Sync employs a lightweight, efficient information-sharing technique that significantly reduces overhead compared to traditional flooding mechanisms.
- **Self-adapting System**: The architecture automatically adjusts to changing network conditions, enhancing both routing performance and security without incurring significant costs.
- **Simulation with NS3**: All proposed mechanisms and techniques are simulated in **NS3**, a discrete-event network simulator.

## Key Components

- **Routing**: A robust routing protocol optimized for reputation-based decisions, increasing the reliability of network paths.
- **Information Sharing**: SAHARA Sync balances the trade-off between performance and resource consumption, especially in mobile environments, ensuring efficient data exchange.
- **Security**: Integrated network security strategies mitigate internal and external attacks, with a focus on minimizing overhead and preserving network resources.

## Use Case

SAHARA is particularly well-suited for **swarm robotics** and **drone networks**, where resource efficiency and security are critical. The system optimizes:

- **Routing robustness**: Better decision-making based on nodes' reputations.
- **Energy efficiency**: Reducing communication costs and energy consumption.
- **Protection against attacks**: Enhanced defenses against both internal and external threats.

## Getting Started

### Prerequisites

- **NS3**: Install NS3 network simulator to run the simulations.
  ```bash
  sudo apt-get install ns-3
  ```

### Installation

Clone this repository:
```bash
git clone https://github.com/your-repo/sahara.git
```

Navigate to the project directory:
```bash
cd sahara
```

### Running Simulations

To execute simulations, follow these steps:

1. Modify the simulation parameters in the `config` file.
2. Run the simulation:
   ```bash
   ./waf --run "sahara-simulation"
   ```

### Output

Simulation results include:

- **Throughput recovery metrics** after attacks.
- **Overhead comparison** with the flooding mechanism.
- **Reputation-based performance metrics** for node trustworthiness.

## Contributions

Contributions are welcome! Feel free to submit pull requests or open issues to improve the project.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
