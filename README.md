# rio2d

rio2d implements a BASIC-like scripting language to help create and test Cocos2d-x actions, supporting live edit of the script code.

A script can have multiple subroutines, and each one of them can be run as a `cocos2d::Action`.

## Compiling scripts

There are two functions that take the script source code and return a `rio2d::Script` instance.

* `static rio2d::Script* rio2d::Script::initWithSource(const char* source, char* error, size_t size);`

Creates an instance of `rio2d::Script` given the script source code. If there were compilation errors, it returns `nullptr`, and copies the error message into `error`, using at most `size` characters including the null terminator.

The `source` string must be null-terminated.

* `static rio2d::Script* rio2d::Script::initWithSource(const char* source);`

Just like the previous function, for when you don't care about the error message.

## Running scripts

* `bool rio2d::Script::runAction(Hash hash, cocos2d::Node* target, ...);`
* `bool rio2d::Script::runAction(const char* name, cocos2d::Node* target, ...);`

Starts the execution on the object `target` of the subroutine whose name's [DJB2](http://www.cse.yorku.ca/~oz/hash.html) hash is `hash`, or use the name directly with the second version. Returns `true` if the subroutine was started, or `false` if it was not found. If the subroutine needs more arguments than the target object, they are passed after the `target` argument of the functions.

Keep in mind that the script is **case-insensitive** when evaluating the DJB2 hash of the subroutine name.

If the subroutine was successfully started, you don't have to do anything else to keep it running.

* `bool rio2d::Script::runActionWithListener(cocos2d::Ref* listener, rio2d::Script::NotifyFunc port, Hash hash, cocos2d::Node* target, ...);`
* `bool rio2d::Script::runActionWithListener(cocos2d::Ref* listener, rio2d::Script::NotifyFunc port, const char* name, cocos2d::Node* target, ...);`

Just like the previous functions, but sets a function that will be notified when the script executes `signal` statements. The function receiving the notification gets the `cocos2d::Node*` instance that is the target of the subroutine, and the DJB2 hash of the string which was the parameter to `signal`.

## Live editing

To implement live editing, use the functions under the `rio2d::Webserver` namespace:

* `bool rio2d::Webserver::init(short port);`

Initialize the embedded web server, making it listen to HTTP request on `port`.

* `void rio2d::Webserver::destroy();`

Stops the web server, releasing all resources. This should be called in your `cocos2d::Application` destructor.

* `rio2d::Script* rio2d::Webserver::getScript(const char* filename);`

Returns a script instance by filename, i.e.

    auto script = rio2d::Webserver::getScript("script.bas");

Instead of keeping the `rio2d::Script` instances yourself, always get them via this function. It does the following:

1. Tries to find a script instance with the same file name, and returns it.
1. Tries to create a new script instance by loading and compiling the source code in the `filename` file.
1. Tries to add a HTTP resource with URL `/<filename>` to the web server (after converting any back slashes to forward slashes).
1. Returns the script.

When the script is successfully registered as a HTTP resource, a HTTP POST to the script's URI will compile the POST data and, if successful, replace the old script instance by the new one, so all calls to `rio2d::Webserver::getScript` will return the new instance. The [cURL](https://curl.haxx.se/) command to POST a new script to the web server is:

    curl --data-binary @scripts.bas "http://192.168.2.5:8080/scripts.bas"

Replace the IP address by the IP of the machine running the application, and change the name of the script. The embedded web server uses the excellent [CivetWeb](https://github.com/civetweb/civetweb).

**Note**: The webserver is removed on release builds (where `NDEBUG` is defined), but you still call `rio2d::Webserver::getScript` to get script instances; you just don't have the ability to update them during runtime anymore.

## Using rio2d in your project

There is no Makefile or Visual Studio solution for rio2d, just copy all files under the `src` folder to your project, preferrably under a folder of their own, and make sure they're compiled along with your own source code.

The easing functions used by the scripts were taken from [AHEasing](https://github.com/warrenm/AHEasing), its source code and also CivetWeb's source code are included here so there's no need to download them.

## DJB2 hashes

All subroutine identifiers are stored as DJB2 hashes to avoid allocating memory for strings. If you use the functions that take the subroutine name, rio2d will evaluate the DJB2 hash of the name on each call. To avoid that, pre-compute the hash and use them instead of subroutine names. There is a small command-line utility to calculate DJB2 hashes in the `etc` folder (compile with `gcc -O2 -o djb2 djb2.c` or a similar command).

    $ djb2 --help
    USAGE: djb2 [ --case ] [ --enum ] [ --prefix ] [ --cpp ] identifiers...

    --case     Lists the hashes with case statements for use with a switch
    --enum     Lists identifiers and the hashes in a format to be used in an enum
    --prefix   Adds a 'k' prefix to the identifiers (when they're used)
    --cpp      Outputs C++-style comments instead of C ones where applicable

## Script syntax

The `example` folder has a script which was used to test the game [Sky Defense](https://www.packtpub.com/game-development/cocos2d-x-example-beginners-guide). The script grammar is:

    script = subroutine* .
    
    subroutine = 'sub' ID '(' ID 'as' 'node' ( ',' ID 'as' type )* ')' statement* 'end' .
    
    type = 'node' | 'number' | 'size' | 'vec2' | 'frames' .
    
    statement = forever | parallel | repeat | sequence | assign | signal | pause .
    
    forever = 'forever' statement* 'end' .
    
    parallel = 'parallel' ( forever | parallel | repeat | sequence )* 'end' .
    
    repeat = 'repeat' expression 'times' statement* 'end' .
    
    sequence = 'sequence' statement* 'end' .
    
    assign = ID '=' expression
           | ID '.' property '=' expression
           | ID '.' method_2 expression ',' expression
           | ID '.' method_3 expression ',' expression ',' expression
           | ID '.' 'setframe' ID ',' expression
           | ID '.' action_0 'in' expression 'secs' ( 'with' ease )?
           | ID '.' action_1 expression 'in' expression 'secs' ( 'with' ease )?
           | ID '.' action_2 expression ',' expression 'in' expression 'secs' ( 'with' ease )?
           | ID '.' action_3 expression ',' expression ',' expression 'in' expression 'secs' ( 'with' ease )?
           .
    
    property = 'bboxheight' | 'bboxwidth' | 'blue' | 'flipx' | 'flipy' | 'green' | 'height' | 'opacity' | 'red'
             | 'rotation' | 'scale' | 'width' | 'x' | 'y' | 'visible'
             .
    
    method_2 = 'place' .
    
    method_3 = 'tint' .
    
    action_0 = 'fadein' | 'fadeout' .
    
    action_1 = 'fadeto' | 'rotateby' | 'rotateto' | 'scaleby' | 'scaleto' .
    
    action_2 = 'moveby' | 'moveto' .
    
    action_3 = 'tintby' | 'tintto' .
    
    ease = 'backin' | 'backinout' | 'backout' | 'bouncein' | 'bounceinout' | 'bounceout' | 'circin' | 'circinout'
         | 'circout' | 'cubicin' | 'cubicinout' | 'cubicout' | 'elasticin' | 'elasticinout' | 'elasticout'
         | 'expin' | 'expinout' | 'expout' | 'linear' | 'quadin' | 'quadinout' | 'quadout' | 'quarticin'
         | 'quarticinout' | 'quarticout' | 'quinticin' | 'quinticinout' | 'quinticout' | 'sinein' | 'sineinout'
         | 'sineout'
         .
    
    signal = 'signal' STR .
    
    pause = 'pause' expression 'secs' .
    
    expression = logica_lor .
    
    logical_or = logical_and ( 'or' logical_and )* .
    
    logical_and = relational ( 'and' relational )* .
    
    relational = term ( ( '=' | '<' | '>' | '<>' | '<=' | '>=' ) term )* .
    
    term = factor ( ( '+' | '-' ) factor )* .
    
    factor = unary ( ( '*' | '/' | 'mod' ) unary )* .
    
    unary = ( '+' | '-' | 'not' )? terminal .
    
    terminal = NUM
             | 'true'
             | 'false'
             | '(' expression ')'
             | ID ( '.' property )?
             | 'rand' ( '(' expression ',' expression ')' )?
             | 'floor' '(' expression ')'
             | 'ceil' '(' expression ')'
             | 'trunc' '(' expression ')'
             .

