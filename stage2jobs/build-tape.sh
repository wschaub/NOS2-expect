#!/bin/bash
../tools/ascTo612 files/konsole rec1
../tools/ascTo612 files/xterm rec2
../tools/ascTo612 files/TTERM.txt rec3
../tools/ascTo612 files/WEBTRM.txt rec4
../tools/NosICreate ../built-tapes/termdefs.tap rec1 rec2 rec3 rec4
../tools/NosIVerify ../built-tapes/termdefs.tap
rm rec?
