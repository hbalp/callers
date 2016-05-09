#!/bin/bash

source fc_analysis.sh

#source fc_parse_preproc_files.hdm.sh > /dev/null 2> .fc_parse.stderr
#source fc_parse_preproc_files.gen.sh > /dev/null 2> .fc_parse.stderr

fc_parse

# emacs -nw fc_parse_preproc_files.gen.sh

# fc_va main 20
fc_va saml_SignatureProfileValidator_validate 20

frama-c-gui -load fc_analyzed.gen.sav &
