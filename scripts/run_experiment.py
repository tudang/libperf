#!/usr/bin/env python
import argparse
import subprocess
import shlex
import os
from threading import Timer
import time
import pandas as pd
import numpy as np
import plotpaxos

def start_proposer(user, host, path, server, speed, output, proid):
    cmd = "ssh {0}@{1} {2}/netpaxos -p -h {3} -d {4}".format(user, host, path, server, speed)
    print cmd
    with open("%s/%s-%d.log" % (output, "proposer", proid), "w+") as out:
        ssh = subprocess.Popen(shlex.split(cmd),
                                stdout = out,
                                stderr = out,
                                shell=False)
    return ssh

def start_learner(user, host, path, output):
    cmd = "ssh {0}@{1} {2}/netpaxos -l".format(user, host, path)
    print cmd
    with open("%s/%s.log" % (output, "learner"), "a+") as out:
        ssh = subprocess.Popen(shlex.split(cmd),
                                stdout = out,
                                stderr = out,
                                shell=False)
    return ssh


def kill_all(*nodes, **parm):
    for h in nodes:
        cmd = "ssh {0}@{1} pkill netpaxos".format(parm['user'], h)
        ssh = subprocess.Popen(shlex.split(cmd))
        ssh.wait()


def copy_data(output, user, nodes):
    for h in nodes:
        cmd = "scp {0}@{1}:*.txt {2}/".format(user, h, output)
        print cmd
        ssh = subprocess.Popen(shlex.split(cmd))
        ssh.wait()


def check_result(output):
    cmd = "diff {0}/learner.txt {0}/proposer.txt".format(output)
    print cmd
    p = subprocess.Popen(shlex.split(cmd),
                        stdout = subprocess.PIPE,
                        stderr = subprocess.PIPE,
                        shell=False)
    out, err = p.communicate()
    if out:
        print out
    if err:
        print err


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Run NetPaxos experiment.')
    parser.add_argument('--time', type=int, default=10, help='amout of time in second to run example')
    parser.add_argument('--speed', type=int, default=10000, help='duration between two proposals')
    parser.add_argument('--multi', type=int, default=2, help='number of client')
    parser.add_argument('--verbose', default=False, action='store_true', help='verbose flag')
    parser.add_argument('--path', default='/libperf/ubuntu', help='path to programs')
    parser.add_argument('--user', default='vagrant', help='login name of ssh')
    parser.add_argument('--learner', default='sdn-vm', help='learner hostname')
    parser.add_argument('--proposer', default='sdn-vm', help='proposer hostname')
    parser.add_argument('--server', default='localhost', help='server hostname')
    parser.add_argument('--output', default='output', help='output folder')
    args = parser.parse_args()

    if not os.path.exists(args.output):
        os.makedirs(args.output)
    nodes = [ args.learner, args.proposer ]
    parm = {'user': args.user}

    pipes = []
    pipes.append(start_learner(args.user, args.learner, args.path, args.output))
    for i in range(args.multi):
        pipes.append(start_proposer(args.user, args.proposer, args.path, 
            args.server, args.speed, args.output, i))
    print "kill replicas and client after %d seconds" % args.time
    t= Timer(args.time, kill_all, nodes, parm)
    t.start()
   
    for p in pipes:
        p.wait()

    print "copy data"
    copy_data(args.output, args.user, nodes)
    # check_result(args.output)
    # plotpaxos.plot_line("%s/learner.log" % args.output, "%s/figure.pdf" % args.output)
