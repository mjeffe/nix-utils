# nix-utils
My bug-ugly collection of tools I install on \*nix boxes

# !!!! USE AT YOUR OWN RISK !!!!
This code is useful to me so I'm making it available to others, but I make no guarantees whatsoever.

-------------------------

This is a collection of small utilities I have created (and in some cases,
maintained), although truthfully there is probably a scattering of some of my
colleagues code in here as well. These utilities are useful for sysadmin,
performance tuning, text processing, database ETL, etc.  It's mostly SQL, C,
Perl, and shell scripts.  Some might even have a helpful usage message :-). A
few were probably just one-offs that I thought might be helpful again sometime.

In some cases (`rrvalue` and `splitval` for example) the code relies on some
proprietary libraries, so you won't be able to compile them. They're included
in the collection because, well... this is my code repo. I'd like to rewrite
them someday, to use an open source equivalent of the libs...

## Install

This is a companion to my [nix-profile](https://github.com/mjeffe/nix-profile) repo. The
`setup.sh` script in that repo will look for and install this repo if it can find it.
Unfortunately, there is little to no documentation on installing dependencies. Usually
what I do is not worry about it unless I need the utility, then I work out what it needs
and install it. For example, if I try to use `adhocexel`, it may complain that the Perl
modules `DBI` and `Spreadsheet::WriteExcel` are not installed. So I install those and
try again.  For the C utilities, you will need the Linux dev tools (`gcc`, `make`, etc)
installed.

Be aware, some of these may not have been updated in years as I've quit using
them, or indeed, their usefulness may be utterly obsolete. This is essentially
an export of the HEAD of my old `subversion` repo, which was an export of the
HEAD of my old `cvs` repo.

