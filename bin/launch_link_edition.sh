#!/bin/bash
#set -x
# Copyright (C) 2015 Thales Communication & Security
#   - All Rights Reserved
# coded by Hugues Balp

common=`which common.sh`
source $common

# List generated json files
#find . -type f -name "*.gen.json.gz" -exec gunzip {} \;
list_files_in_dirs `pwd` .file.callers.gen.json dir.callers.gen.json ".libs:.deps:all:doc:msbuild:m4:build-aux:autom4te.cache:demos:samples:Samples:configs:cxxtestplus-0.3:.git:projects:Projects:ref:tests:config"

# List all defined symbols in file defined_symbols.json
list_defined_symbols defined_symbols.json `pwd` dir.callers.gen.json
#read_defined_symbols.native defined_symbols.json file.callers.gen.json

# add extcallees to json files
source add_extcallees.sh `pwd`

# add extcallers to json files
source add_extcallers.sh .
source indent_jsonfiles.sh .
