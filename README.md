# Tcppop

Tcppop is a program for generating spoof network traffic within linux kernel and then send them to nic.

## Architecture

```
              ---------------------------------------------------


user space


              ---------------------------------------------------

kernel space                       
                                   |  |
              ---------------------|--|--------------------------
                                +--|--|--+
                                |        |
                                |   NIC  |
                                |        |
                                +--------+  

```
## Build
```
make
```

## Usage
```
./tcppop -l 4 -sa 192.201.0.1 -da 192.201.0.2 -sp 1234 -dp 22 -c payload.txt
```







