[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://github.com/theblackoreo/smesh/blob/main/LICENSE)

![Python](https://img.shields.io/badge/python-3670A0?style=for-the-badge&logo=python&logoColor=ffdd54)
![C++](https://img.shields.io/badge/-C++-blue?logo=cplusplus)



# SAHARA: Secure Ad-Hoc Advanced Resilient Architecture

## Overview

SAHARA (Secure Ad-Hoc Advanced Resilient Architecture) is a collaborative reputation-based system designed to enhance routing decisions and strengthen network security in mobile ad-hoc networks (MANETs) with minimal overhead. This system integrates three key concepts:

- **Routing Protocols**
- **Information Dissemination**
- **Network Integrity (security)**

The system dynamically adapts to changes in the network, providing robust security against common attacks while minimizing the impact on resource-constrained devices, such as those in **robot** and **drone swarms**.

## Features

- **Information dissemination**: SAHARA Sync employs a lightweight, efficient information-sharing technique that significantly reduces overhead compared to traditional flooding mechanisms.
- **Reputation-based Node Evaluation**: SAHARA leverages reputation mechanisms to assess node trustworthiness, ensuring that only reliable nodes participate in routing decisions.
- **Resilience to Attacks**: The system dynamically responds to malicious behaviors (e.g., packet dropping, DoS attacks), ensuring network throughput recovery in hostile environments.
- **Self-adapting System**: The architecture automatically adjusts to changing network conditions, enhancing both routing performance and security without incurring significant costs.
- **Simulation with NS3**: All proposed mechanisms and techniques are simulated in **NS3**, a discrete-event network simulator.

## Use Case

SAHARA is particularly well-suited for **swarm robotics** and **drone networks**, where resource efficiency and security are critical. The system optimizes:

- **Routing robustness**: Better decision-making based on nodes' reputations.
- **Energy efficiency**: Reducing communication costs and energy consumption.
- **Protection against internal attacks aim to destroy network integrity**: Enhanced defenses against both internal threats.

## Getting Started

### Prerequisites

- **NS3**: Install NS3 network simulator. [Guide](https://www.nsnam.org/docs/installation/html/)

### Installation
- **Download the repository and copy SAHARA module into the NS3 module directory** [Guide](https://www.nsnam.org/docs/manual/html/new-modules.html)
- **Install required libraries**: [PyViz](https://www.nsnam.org/wiki/PyViz) and [CRYPTOPP](https://www.projectguideline.com/secrets-of-using-cryptography-in-ns-3-using-crypto-library/) 

**NOTE:** Configure NS3 in order to enable examples, tests, visualizer, mesh, sahara and tests. [Guide](https://www.nsnam.org/docs/tutorial/html/getting-started.html)

### SAHARA NS3 MODEL

The Sahara NS3 Model is divided into different modules, each managing a specific part of the framework. The main module is *saharaRouting.c* [here](https://github.com/theblackoreo/smesh/blob/main/ns3_simul/Sahara4.0/Model/saharaRouting.cc), which serves as the core and integrates the use of the other sub-modules. Starting from the *saharaRouting.h* file, it is possible to enable or disable various framework features such as an improved version of Flooding, static and dynamic SAHARA modes, and the option to activate a specific malicious node. Additionally, many more functions can be configured within each sub-module file.

# SAHARA NS3 SIMULATIONS
Custom simulations can be run by modifying one of the simulation files provided, which have been used to evaluate some statistics. These files are only examples and can be customized by including more functions, additional rules for message transmission, and/or methods to evaluate SAHARA performance.

Additionally, these files offer the possibility to enable debugging functions that print each operation performed by the SAHARA modules. Alternatively, the mobility model can be activated. An important feature is the ability to change the network topology and the number of nodes. Note that the maximum number of nodes currently supported is 255. The saharaRouting file may need to be modified to better manage the timing of each SAHARA phase; otherwise, the simulation might fail.


## Contributions

Contributions are welcome! Feel free to submit pull requests or open issues to improve the project.

## License

This project is licensed under the MIT License - see the [LICENSE](https://github.com/theblackoreo/smesh/blob/main/LICENSE) file for details.
