let s:plugin_root = expand('<sfile>:p:h:h')
let s:binary = s:plugin_root . '/neobolt_server'

function! neobolt#init_plug()
  if exists('g:neobolt')
    return
  endif
  if !exists('s:jobId')
    let s:jobId = 0
  endif

  let l:id = s:run_backend()

  if l:id == 0
    echoerr "Neobolt: cannot start rpc process"
    return
  elseif l:id == -1
    echoerr "Neobolt: given backend is not executable"
    return
  end

  let s:jobId = l:id
  call s:configure_commands()

  augroup neobolt
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
