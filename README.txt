#################################################################
#  _____   _____               _       __           _        _  #
# |  __ \ / ____|  /\         | |     / _|         | |      ( ) #
# | |__) | (___   /  \      __| | ___| |_ __ _  ___| |_ ___ |/  #
# |  _  / \___ \ / /\ \    / _` |/ _ \  _/ _` |/ __| __/ _ \    #
# | | \ \ ____) / ____ \  | (_| |  __/ || (_| | (__| || (_) |   #
# |_|  \_\_____/_/    \_\  \__,_|\___|_| \__,_|\___|\__\___/    #
#                                                               #
#################################################################

RSA defacto' – Proof-of-concept 
RSA public key (semi prime) factoring algorithm

Copyright 2016 Goldcove a.k.a. 2f426c5f72137f8ba47eb4db48c475c98ee5ba82de1f555b7902d2c5975fa2a7 (sha256). 
This work is licensed under a GPLv3 or later license. 

This has been a fun little project but it's time to share it with the world.

Basic operation:
RSA public key is the product of p x q (modulus) and is a semi prime.
We're basically looking for an area as described by the "rectangle" p x q. 
This is area is expressed by 'x' times 'y' and a carry ('c') in this algorithm.
For every iteration we're subtracting 2y, shown as 'a', (we are only interested in odd numbers) and adding carry to and incrementing x. The goal is to find a 'x' times 'y' combo which
results in a zero carry, i.e. we've found the primes p and q.

Why is this faster than say trial division?
The 'y' and carry can be large enough to allow us to skip several x and thereby skip several computations. 
As y decrease we'll be able to skip more and more x. 

Is this more efficient than other techniques?
To be quite frank, I have no idea. I have not had the opportunity to really test my C implementation as I would need more 
powerful hardware.  
My biggest concern is that the cost of division and addition on large numbers will outweigh this technique's benefits. The production code implementation would need code and possible hardware optimization to maximize performance. 
On the other hand can this technique be used in parallel by creating several different starting rectangles.
Eventually I might do like Carsten Nohl and test it in the cloud... and then all your semi prime are belong to me (maybe)!

The following example will visaully show the technique:
Iteration 0
We start with a (close to) square (X==Y) and a carry, ie X * Y + carry == semi prime
^ * * * * 
| * * * * * * * *
| * * * * * * * *
| * * * * * * * *
y * * * * * * * *
| * * * * * * * *
| * * * * * * * *
| * * * * * * * *
| * * * * * * * *
  ------ x ----->

Iteration 1
 c c c c  <-Carry
 a a a a a a a a  >  
 a a a a a a a a  >                   
 * * * * * * * *  >  * * * * * * * * a a
 * * * * * * * *  >  * * * * * * * * a a c
 * * * * * * * *  >  * * * * * * * * a a c
 * * * * * * * *  >  * * * * * * * * a a c
 * * * * * * * *  >  * * * * * * * * a a c
 * * * * * * * *  >  * * * * * * * * a a a
 * * * * * * * *  >  * * * * * * * * a a a
 Our new carry is 6

Iteration 2
 c c c c c c 
 a a a a a a a a a a  >
 a a a a a a a a a a  >
 * * * * * * * * * *  >  * * * * * * * * * * a a c
 * * * * * * * * * *  >  * * * * * * * * * * a a a
 * * * * * * * * * *  >  * * * * * * * * * * a a a c
 * * * * * * * * * *  >  * * * * * * * * * * a a a c
 * * * * * * * * * *  >  * * * * * * * * * * a a a c
 * * * * * * * * * *  >  * * * * * * * * * * a a a c
 * * * * * * * * * *  >  * * * * * * * * * * a a a c
 Our new carry is 5

 ...


Iteration n
 c c c
 a a a a a a a a a a a a  >  
 a a a a a a a a a a a a  >  
 * * * * * * * * * * * *  >  * * * * * * * * * * * * a a a a a a a a c
 * * * * * * * * * * * *  >  * * * * * * * * * * * * a a a a a a a a c
 * * * * * * * * * * * *  >  * * * * * * * * * * * * a a a a a a a a c
Observe that we no longer have a carry! We've found a rectangle that match our semi prime!

Additional info:
How to get RSA public key modulus from TLS cert:
openssl x509 -pubkey -noout < /tmp/rsasslcert.from.server.com.txt |openssl rsa -pubin -noout -modulus
Extract GPG/PGP RSA public key info: 
gpg -a --export DEADBEEEEF|pgpdump -i
