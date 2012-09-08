# Fast Compiliing C++ Library
-----------------------------------------

In my prior attempts at developing MACE what I discovered is that compile times
would explode to unreasonable levels that hinder the rate of development more
than what can be saved by reduced typing.  So I began a quest to get C++ to compile 
as quickly as Java or C# and the result is this library.

One of the major drawbacks to templates is that they place everything in header and
must be compiled with every run and generate a lot of object code.  With Link Time Optimization,
the benefit of inline methods mostly disapears, leaving only static vs dynamic polymorphism.

For the vast majority of applications, a virtual method call is not the bottleneck and the
increased compile times costs more than is otherwise justified; therefore, the Fast Compiling C++ 
library opts for virtual interfaces to handle reflection instead of template interfaces.  One could
argue that both types of reflection could be useful.

Another source of slowness was the standard template library itself.  Most standard template library
classes cannot be forward declared and import thousands of lines of code into every compilation unit.

Another source of slowness is the need to include headers simply because the 'size' of the object must
be known.  A new utility class allows you to 'forward declare' the size required for certain types which
allows you to remove their inclusion from the header file.


