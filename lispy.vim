" Vim syntax file
" Language: Lispy
" Maintainer: Pavel Sergeev
" Latest Revision: 07 February 2016

if exists("b:current_syntax")
  finish
endif

syn match lispyComment ";.*$"
syn keyword lispyBuiltinFunctions list head tail join eval def print if fun macro
syn match lispyNumber '\d\+'
syn match lispyNumber '[-+]\d\+'

let b:current_syntax = "lispy"
hi def link lispyComment Comment
hi def link lispyBuiltinFunctions Statement
hi def link lispyNumber Constant

