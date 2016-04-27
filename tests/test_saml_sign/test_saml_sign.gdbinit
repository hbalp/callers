shell rm test_saml_sign.gen.gdb.stdout
set pagination off
set logging file test_saml_sign.gen.gdb.stdout
set logging overwrite on

file build/test_saml_sign

# init of local some variables
set var $min_a=0
set var $max_a=0
set var $min_b=0
set var $max_b=0

b division

set logging on
printf "a, b\n"
set logging off

set logging overwrite off

commands
if a < $min_a
 set $min_a=a
end
if a > $max_a
 set $max_a=a
end
if b < $min_b
 set $min_b=b
end
if b > $max_b
 set $max_b=b
end
set logging on
printf "%d, %d\n", a , b
set logging off
#bt full
c
end

#info br
r
set logging on
printf "min_a, max_a, min_b, max_b\n"
printf "%d, %d, %d, %d\n", $min_a , $max_a, $min_b, $max_b
set logging off
quit
