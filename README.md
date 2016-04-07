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

Starts the execution on the object `target` of the subroutine whose name's [DJB2](http://www.cse.yorku.ca/~oz/hash.html) hash is `hash`, or use the name directly with the second version. Returns `true` if the subroutine was started, or `false` if it was not found. If the subroutine needs more arguments than the target object, they are passed after the `target` argument of this function.

Keep in mind that the script is **case-insensitive** when evaluating the DJB2 hash of the subroutine name.

If the subroutine was successfully started, you don't have to do anything else to keep it running.

* `bool rio2d::Script::runActionWithListener(cocos2d::Ref* listener, rio2d::Script::NotifyFunc port, Hash hash, cocos2d::Node* target, ...);`
* `bool rio2d::Script::runActionWithListener(cocos2d::Ref* listener, rio2d::Script::NotifyFunc port, const char* name, cocos2d::Node* target, ...);`

Just like the previous functions, but sets a function that will be notified when the script executes `signal` statements. The function receiving the notification gets the `cocos2d::Node*` instance that is the target of the subroutine, and the DJB2 hash of the string which was the parameter to `signal`.

## Using rio2d in your project

There is no Makefile or Visual Studio solution for rio2d, just add `rio2d.h`, `script.cpp`, and `easing.inl` to your project to be compiled along with your own source code, preferrably under a folder of their own.

The easing functions used by the scripts were taken from [AHEasing](https://github.com/warrenm/AHEasing).

## Live editing

Live editing can be easily achieved with the use of an embedded webserver like [civetweb](https://github.com/civetweb/civetweb). The *example* folder has source code released in the public domain showing how to embed civetweb to implement live editing. Use [cURL](https://curl.haxx.se/) or another utility that can make POST requests to change the script anytime:

`curl --data-binary @scripts.bas "http://192.168.2.5:8080/scripts.bas"`

## Script syntax

The *example* folder has a script which was used to test the game [Sky Defense](https://www.packtpub.com/game-development/cocos2d-x-example-beginners-guide). The script grammar is:

    script = subroutine* .
    
    subroutine = 'sub' ID '(' ID 'as' 'node' ( ',' ID 'as' type )* ')' statement* 'end' .
    
    type = 'node' | 'number' | 'size' | 'vec2' .
    
    statement = forever | parallel | repeat | sequence | set_prop | assign | signal | pause .
    
    forever = 'forever' statement* 'end' .
    
    parallel = 'parallel' ( forever | parallel | repeat | sequence )* 'end' .
    
    repeat = 'repeat' expression statement* 'end' .
    
    sequence = 'sequence' statement* 'end' .
    
    set_prop = ID '.' ID '=' expression
             | ID '.' ( fadein | fadeout ) 'in' expression 'secs' ( 'with' ease )?
             | ID '.' ( rotateby | rotateto | scaleby | scaleto ) expression 'in' expression 'secs' ( 'with' ease )?
             | ID '.' ( moveby | moveto ) expression ',' expression 'in' expression 'secs' ( 'with' ease )?
             .
    
    ease = backin | backinout | backout | bouncein | bounceinout | bounceout | circin | circinout | circout
         | cubicin | cubicinout | cubicout | elasticin | elasticinout | elasticout | expin | expinout | expout
         | linear | quadin | quadinout | quadout | quarticin | quarticinout | quarticout
         | quinticin | quinticinout | quinticout | sinein | sineinout | sineout
         .
    
    assign = 'let' ID '=' expression .
    
    signal = 'signal' STR .
    
    pause = 'pause' expression .
    
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
             | ID ( '.' ID )?
             | 'rand' ( '(' expression ',' expression ')' )?
             | 'floor' '(' expression ')'
             | 'ceil' '(' expression ')'
             | 'trunc' '(' expression ')'
             .

