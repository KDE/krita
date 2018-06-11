alias ll='ls -alF'
alias la='ls -A'
alias l='ls -CF'

alias ..='cd ..'
alias ...='cd ../..'
alias ....='cd ../../..'

alias gdba='gdb --args'
alias gdbr='gdb --eval-command=r --args'
alias gdbrk='gdb --eval-command=r --args krita'
alias gdbatt='gdb --eval-command=cont --pid=$(pidof krita)'

function tst {
    make -j8 $1 && ./$1 $2 $3 $4 $5 $6 $7 $8
}

function tsti {
    make -j8 -C .. && make -j1 -C .. install/fast && ./$1 $2 $3 $4 $5 $6 $7 $8
}

function tstfast {
    make -j8 $1/fast && ./$1 $2 $3 $4 $5 $6 $7 $8
}

function mi {
    make -j8 && make -j1 install/fast
}

function mik {
    make -j8 && make -j1 install/fast && krita $*
}

function nmik {
    nice make -j8 && nice make -j1 install/fast && krita $*
}


