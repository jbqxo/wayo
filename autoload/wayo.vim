"  Copyright (c) 2019 Maxim Lyapin
"
"  Permission is hereby granted, free of charge, to any person obtaining a copy
"  of this software and associated documentation files (the "Software"), to deal
"  in the Software without restriction, including without limitation the rights
"  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
"  copies of the Software, and to permit persons to whom the Software is
"  furnished to do so, subject to the following conditions:
"
"  The above copyright notice and this permission notice shall be included in all
"  copies or substantial portions of the Software.
"
"  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
"  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
"  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
"  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
"  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
"  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
"  SOFTWARE.
"

let s:plugin_root = expand('<sfile>:p:h:h')
let s:binary = s:plugin_root . '/wayo_server'

function! wayo#init_plug()
    if exists('g:wayo')
	return
    endif
    if !exists('s:jobId')
	let s:jobId = 0
    endif

    let l:id = s:run_backend()

    if l:id == 0
	echoerr "WAYO: cannot start rpc process"
	return
    elseif l:id == -1
	echoerr "WAYO: given backend is not executable"
	return
    end

    let s:jobId = l:id
    call s:configure_commands()

    augroup wayo
	autocmd!
    augroup END
endfunction

function! s:run_backend()
    if s:jobId == 0
	let l:id = jobstart([s:binary], { 'rpc': v:true, 'on_stderr': function('s:OnStderr')})
	return l:id
    else
	return s:jobId
    endif
endfunction

function! s:configure_commands()
    command! -nargs=0 NeoboltDisassm :call rpcnotify(s:jobId, "disassemble")
endfunction

function! s:OnStderr(id, data, event) dict
    echoerr 'stderr(' . a:event . '): ' . join(a:data, "\n")
endfunction
