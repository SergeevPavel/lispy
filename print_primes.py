#!/bin/bash/python

def is_prime(x):
    d = 2
    r = True
    while (d * d < x) and r:
        if x % d:
            pass
        else:
            r = False
        d += 1
    return r
            
def print_primes(m):
    for i in range(m):
        if is_prime(i):
            print(i)

def main():
    print_primes(1000)

if __name__ == "__main__":
    main()
