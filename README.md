# ping

Ping CLI application written in C++ using boost/asio and cxxopts.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine.

### Prerequisites

Make sure you have g++ and make, and you are on MacOS or Linux.

### Installing

Clone this repository (use the `--recursive` flag as it contains submodules)

```bash
git clone https://github.com/dniwrallets/ping.git --recursive
```

Navigate to the root and run make, the final executable (`ping`) will be in the `out` direcotry.

```bash
make
```

Run the program (note: you may need to be root, do `--help` for more options)

```bash
sudo out/ping google.com
172.217.8.206:0: seq=0, time=14.372ms:
1 packets transmitted, 0.000% loss
min/avg/max/ = 14.372/14.372/14.372 ms

172.217.8.206:0: seq=1, time=18.273ms:
2 packets transmitted, 0.000% loss
min/avg/max/ = 14.372/16.322/18.273 ms

...
```

## Built With

- [boost/asio](https://think-async.com/Asio/) - For network programming
- [cxxopts](https://github.com/jarro2783/cxxopts) - For command line option parsing
- [make](https://www.gnu.org/software/make/) - For executable generation

## Author

- **John Yang** - [John](https://github.com/dniwrallets)

## Acknowledgments

- **Chris Kohlhoff** - boost/asio - [chriskohlhoff](https://github.com/chriskohlhoff)
- **jarro2783** - cxxopt - [jarro2783](https://github.com/jarro2783)
- **Billie Thompson** - README.md template - [PurpleBooth](https://github.com/PurpleBooth)
