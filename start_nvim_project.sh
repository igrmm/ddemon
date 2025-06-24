#!/bin/sh

cd src
nvim \
    -c "set makeprg=cd\ ..\ &&\ sh\ run.sh" \
    -c "map <F5> :make<CR><CR>" \
    -c ":NoNeckPain" \
    .
