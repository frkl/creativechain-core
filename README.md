<p align="center">
  <br>
  <a ><img src="http://creativechain.org/img/crea1.jpg" alt="" width="100%"></a>
  <br>
<p align="center">
  <a href="http://creativechain.org/"><img src="http://creativechain.org/img/b-creativechain.jpg" alt="Gitter"></a>
  <p align="center">
  <a href=""><img src="http://creativechain.org/img/crea2.jpg" alt="Gitter"></a>
  <p align="center">
  <a href="http://creativechain.org/roadmap/"><img src="http://creativechain.org/img/b-roadmap.jpg" alt="Gitter"></a>
  <p align="center">
  <a href=""><img src="http://creativechain.org/img/crea3.jpg" alt="Gitter"></a>
  <p align="center">
  <a href="http://creativechain.org/ico/"><img src="http://creativechain.org/img/b-ico.jpg" alt="Gitter"></a>
  <p align="center">
  <a href=""><img src="http://creativechain.org/img/crea4.jpg" alt="Gitter"></a>
  <p align="center">
  <a href="http://creativechain.org/whitepaper/"><img src="http://creativechain.org/img/b-whitepaper.jpg" alt="Gitter"></a>
  <p align="center">
  <a href=""><img src="http://creativechain.org/img/crea5.jpg" alt="Gitter"></a>

<p align="center">
  <a href="https://github.com/creativechain/creativechain-core"><img src="http://creativechain.org/img/b-MAC.jpg" alt="Gitter"></a>
  <a href="https://github.com/creativechain/creativechain-core"><img src="http://creativechain.org/img/b-WIN.jpg" alt="Travis Build"></a>
  <a href="https://github.com/creativechain/creativechain-core"><img src="http://creativechain.org/img/b-LINUX.jpg"></a>
</p>
<br>

<p align="center">
  <a href="http://creativechain.org/"><img src="http://creativechain.org/img/b-media-explorer.jpg"></a>
  <a href="https://github.com/creativechain/creativechain-core"><img src="http://creativechain.org/img/b-block-explorer.jpg"></a>
 </p>
 
  
<h3 align="center">http://creativechain.org/ico/</h3>

<h3 align="center">http://creativechain.org/whitepaper/</h3>






License
-------

Creativecoin Core is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see https://opensource.org/licenses/MIT.

Development Process
-------------------

Developer IRC can be found on Freenode at #creativecoin #creativechain.

Testing
-------

Testing and code review is the bottleneck for development; we get more pull
requests than we can review and test on short notice. Please be patient and help out by testing
other people's pull requests, and remember this is a security-critical project where any mistake might cost people
lots of money.

### Automated Testing

Developers are strongly encouraged to write [unit tests](/doc/unit-tests.md) for new code, and to
submit new unit tests for old code. Unit tests can be compiled and run
(assuming they weren't disabled in configure) with: `make check`

There are also [regression and integration tests](/qa) of the RPC interface, written
in Python, that are run automatically on the build server.
These tests can be run (if the [test dependencies](/qa) are installed) with: `qa/pull-tester/rpc-tests.py`

The Travis CI system makes sure that every pull request is built for Windows, Linux, and OS X, and that unit/sanity tests are run automatically.

### Manual Quality Assurance (QA) Testing

Changes should be tested by somebody other than the developer who wrote the
code. This is especially important for large or high-risk changes. It is useful
to add a test plan to the pull request description if testing the changes is
not straightforward.
