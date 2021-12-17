# hbst-python

Python bindings and pre-compiled binaries for the [Hamming Binary Search Tree (HBST)](https://gitlab.com/srrg-software/srrg_hbst)
for visual place recognition. The HBST algorithm and [paper](https://arxiv.org/abs/1802.09261) is not my work. If you use this library please cite the authors:

```
@article{2018-schlegel-hbst, 
  author  = {D. Schlegel and G. Grisetti}, 
  journal = {IEEE Robotics and Automation Letters}, 
  title   = {{HBST: A Hamming Distance Embedding Binary Search Tree for Feature-Based Visual Place Recognition}}, 
  year    = {2018}, 
  volume  = {3}, 
  number  = {4}, 
  pages   = {3741-3748}
}
```


## Installation and Usage

```bash
pip install hbst-python
```

Note that currently only Linux x86_64 is supported.

Check out the [example](example/match.py) to get started with OpenCV descriptors and HBST.


## Development

Development use cases are dockerized. The commands below will spawn the respective docker container mount the repo inside and do the job.

Syntax:
```bash
./build.sh [platform] [python-version]
./test.sh [platform] [python-version]
```

Example:
```bash
./build.sh linux_x86_64 3.8
./test.sh linux_x86_64 3.8
```
