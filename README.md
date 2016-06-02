# ex-3.7 (from 4.4BSD release)
This is the final release of the original `vi` taken from 4.4BSD.
It had been released in 1993 but the last functional changes had been made in 1985,
hence `:ve` gives this date.
## Installation notes
Some configuration (e.g. installation paths) can be done in the
[`makefile`](https://github.com/n-t-roff/ex-3.7_4.4BSD/blob/master/Makefile.in).
For compiling it on BSD, Linux and Solaris autoconfiguration is required:
```sh
$ ./configure
```
The software is build with
```sh
$ make
```
and installed with
```
$ su
# make install
# exit
```
All generated files are removed with
```sh
$ make distclean
```
## Usage notes
* The original `vi` never had a `showmode` option.
  (`showmode` in Solaris and Heirloom `vi` is no original code.)
* PAGE-UP, PAGE-DOWN keys may work on most terminals by putting
  `map  ^[[5~ ^B` and `map  ^[[6~ ^F` into `~/.exrc`.
  If this doesn't work on your terminal you may need other escape sequences which can be retrieved with
  `infocmp -l` from capabilities `kpp` and `knp`.

**Attention**:
The original `vi` had not been 8-bit clean!
Moreover it does automatically change all 8-bit characters to 7-bit in the whole file even if no editing is done!
This will e.g. destroy all UTF-8 characters.
`vi` gives you a hint in this case by requiring `:w!` for writing even if you have UNIX write permissions
(so be warned if `vi` asks for `:w!` unexpectedly).