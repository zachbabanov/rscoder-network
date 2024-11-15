# Reed Solomon Network Tool

Point-to-Point UDP transmission with error correction by Reed Solomon Codes

## Overview

Implementation was designed for POSIX systems, tested at Debian 11 and Ubuntu 22.04<br>
Suitable for localhost, Docker containers or public network

## Dependencies

For build and compile program you will need CMake, clang and gcc.<br>
Check if they are installed on your system or get them by `apt-get install`:

```shell script
sudo apt-get install cmake clang gcc
```

Readline library is necessary for Receiver Point

```shell script
sudo apt-get install libreadline-dev
```

Every other dependency is either system library or provided by repo

## Installation and build

For proper installation you will need to copy repository with it submodules:

```shell script
git clone --recurse-submodules https://github.com/zachbabanov/rscoder-network
```

After that you will need to build a required point. It is possible to run both points on the same machine,<br>
but for real network results with possible losses and error correcting recommended to run them on separate<br>
devices within one network. You can simulate noises and losses in that network by appropriate software

Building the project follows next steps:

* Prepare directory with CMake

```shell script
sudo cmake CMakeLists.txt
``` 

* Build point executable
```shell script
sudo make receiver
```
or

```shell script
sudo make sender
```

Both command will also create two directories within current, named `./csv` and `./files`. First is used for<br>
storing statistic files, second for transmitted files received by point

## Usage

Receiver Point always must be started first. If you are running root user do not forget `sudo` with executable.<br>
Receiver Point executable need to be started with provided `PORT_NUMBER`

```shell script
sudo ./receiver 8080
```

After that you will get info about `IPv4 Address` used by Receiver Point. It must be provided to start Sender Point<br>
as well with same `PORT_NUMBER`

```shell script
sudo ./sender 8080 127.0.1.1
```

Sender Point will provide a simple CLI to operate

### Commands

Sender point has few command to interact with.

* `bench` command to obtain information about initial state of used network

```shell script
sender_point>bench -b BANDWIDTH (optional)-n AMOUNT
```

`bench` command has two flags: `-b` to specify packet bandwidth and `-n` to specify amount of all packets sent.<br>
`-b` flag is essential and must be followed by integer number in range `(1, 64)`<br>
`-n` flag is optional, by default number of packets sent is `2000`. Flag must be followed by integer number in range (1, 100000)<br>

* `send` command to transmit actual file to connected Receiver Point
```shell script
sender_point>send -f FILEPATH -b BANDWIDTH
```

`send` command has two essential flags: `-b` to specify packet bandwidth and `-f` to provide a file path to sending file<br>
`-f` flag must be followed by valid file path. At the moment only files considered as text info (`.txt`, `.csv`, `.conf`,<br>
`.sh`, `bat`, `.svg` etc) can be transited properly
`-b` flag is essential and must be followed by integer number in range `(1, 64)`<br>
It represents number of data block stored in single package, i. e. with `-b 1` it will be only one `255 bytes` block
`send` command also have two optional flags: `-l` to continuously transmit single file in a loop and `-n` to send file 
without encoding<br>
`-l` flag, if used, must be followed by integer number in range `(1, 100000)`<br>
It represents a number of cycles single file will be transmitted in a loop<br>
`-n` flag has no argument to be followed by must be used as it is. If flag used, all messages will be twice as big and 
will be transmitted without encoding<br>

* `quit` or `q` command to terminate program

### Statistics

Both Sender and Receiver points produce statistical data based on result of each data transmission.<br>
All statistics provided in `.csv` files in namesake directory.<br>

Files generated by benchmarking named `TIMESTAMP_receiver_bench_output.csv`<br>
and `TIMESTAMP_sender_bench_output.csv`, files generated by sending a file named `TIMESTAMP_receiver_send_output.csv`<br>
and `TIMESTAMP_sender_send_output.csv`. All files have next header structure:

| File number `for send` |  Packet Number  |  Bandwidth  |  Timestamp  |  Number of errors  |
|------------------------|-----------------|-------------|-------------|--------------------|

## Known issues

|  #  |                      Issue                   |          Solved          |
|-----|----------------------------------------------|--------------------------|
|  1  |     Transmitting only text handled files     | <ul><li>- [ ] </li></ul> |
|  2  |    Overflowing decoding with bandwidth > 1   | <ul><li>- [x] </li></ul> |
