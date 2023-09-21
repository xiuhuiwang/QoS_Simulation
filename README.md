# QoS_Simulation
Implement Strict Priority Queue and Deficit Round Robin, and simulate with NS-3

## How to run

Put the project folder under ns3/scratch
Put the config file under ns3 root folder

- cd to the root, i.e. ns-3-dev

You may run SPQ validation by:
- ./ns3 run spq-validation -- --filename=config-spq.txt
where config.txt is the name of your config file

You may run DRR validion by:
- ./ns3 run drr-validation -- --filename=config-drr.txt
