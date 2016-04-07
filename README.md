# rio2d

rio2d implements a BASIC-like scripting language to help create and test Cocos2d-x actions, supporting live edit of the script code.

A script can have multiple subroutines, and each one of them can be run as a `cocos2d::Action`.

## Compiling scripts

There are two functions that take the script source code and return a `rio2d::Script` instance.

* `static rio2d::Script* rio2d::Script::initWithSource(const char* source, char* error, size_t size);`

Creates an instance of `rio2d::Script` given the script source code. If there were compilation errors, it returns `nullptr`, and copies the error message into `error`, using at most `size` characters including the null terminator.

* `static rio2d::Script* rio2d::Script::initWithSource(const char* source);`

Just like the previous function, for when you don't care about the error message.

## Running scripts

* `bool rio2d::Script::runAction(Hash hash, cocos2d::Node* target, ...);`
* `bool rio2d::Script::runAction(const char* name, cocos2d::Node* target, ...);`

Starts the execution on the object `target` of the subroutine whose name's [DJB2](http://www.cse.yorku.ca/~oz/hash.html) hash is `hash`, or use the name directly with the second version. Returns `true` if the subroutine was started, or `false` if it was not found. If the subroutine needs more arguments than the target object, they are passed after the `target` argument of this function.

Keep in mind that the script is case-insensitive when evaluating the DJB2 hash of the subroutine name.

If the subroutine was successfully started, you don't have to do anything else to keep it running.

* `bool rio2d::Script::runActionWithListener(cocos2d::Ref* listener, rio2d::Script::NotifyFunc port, Hash hash, cocos2d::Node* target, ...);`
* `bool rio2d::Script::runActionWithListener(cocos2d::Ref* listener, rio2d::Script::NotifyFunc port, const char* name, cocos2d::Node* target, ...);`

Just like the previous function2, but sets a function that will be notified when the script executes `signal` statements. The function receiving the notification gets the `cocos2d::Node*` instance that is the target of the subroutine, and the DJB2 hash of the string which was the parameter to `signal`.

## Live editing

Live editing can be easily achieved with the use of an embedded webserver like [civetweb](https://github.com/civetweb/civetweb). The *example* folder has an example, released in the public domain.

## Script Syntax

The *example* subfolder has a script which was used to test the game [Sky Defense](https://www.packtpub.com/game-development/cocos2d-x-example-beginners-guide). The script grammar is:

    script = subroutine* .
    
    subroutine = ID '(' ID 'as' 'node' ( ',' ID 'as' type )* ')' statement* 'end' .
    
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

# Easing Functions

The easing functions used by the scripts were taken from [AHEasing](https://github.com/warrenm/AHEasing). Just add `easing.h` and `easing.c` to your projetcs, taking measures to guarantee that the number type used if `float`, not `double`.
