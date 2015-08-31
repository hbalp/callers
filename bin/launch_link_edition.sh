#!/bin/bash
#set -x

# List generated json files
find . -type f -name "*.gen.json.gz" -exec gunzip {} \;
list_files_in_dirs `pwd` .file.callers.gen.json dir.callers.gen.json

# List all defined symbols in file defined_symbols.json
list_defined_symbols defined_symbols.json `pwd` dir.callers.gen.json
#read_defined_symbols.native defined_symbols.json file.callers.gen.json

# add extcallees to json files
source add_extcallees.sh `pwd` defined_symbols.json

# add extcallers to json files
source add_extcallers.sh .
source indent_jsonfiles.sh .
