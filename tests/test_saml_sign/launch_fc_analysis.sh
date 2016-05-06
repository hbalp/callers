#!/bin/bash

source fc_parse_preproc_files.hdm.sh > /dev/null 2> .fc_parse.stderr

source fc_analysis.sh

fc_va 20
