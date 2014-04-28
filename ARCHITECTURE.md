Architecture
============


# IO

The library presents an IO layer which can be plugged in an evented IO library of choice, such as
libuv.


The library is organized as a state machine that reacts to input IO events, and to a worker function
which is expected to return in a few hundreds of miliseconds which is expected to be called
periodically in the idle callback of the even loop until it returns false.


## Input IO

Input IO is sent to the library via ProtocolState::input.

## Output IO

The "do_write" function provided to ProtocolState::set_write_fun will be used to write data, typically to write
data in the specific way that the chose asynchronous IO library requires. When the write has
finished, the function ProtocolState::on_write_finished should be called. 

Then either the next write will be scheduled by calling "do_write" again, or nothing will happen if
there's no more data to be writen.
o



# Messages


The class ProtocolState is responsible for reading data until a full message or a payload chunk can
be assembled.


