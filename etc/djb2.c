#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

static void showhelp( FILE* out )
{
  fprintf( out, "USAGE: djb2 [ --case ] [ --enum ] [ --prefix ] [ --cpp ] identifiers...\n\n" );
  fprintf( out, "--case     Lists the hashes with case statements for use with a switch\n" );
  fprintf( out, "--enum     Lists identifiers and the hashes in a format to be used in an enum\n" );
  fprintf( out, "--prefix   Adds a 'k' prefix to the identifiers (when they're used)\n" );
  fprintf( out, "--cpp      Outputs C++-style comments instead of C ones where applicable\n\n" );
}

static uint32_t djb2( const char* str )
{
  uint32_t hash = 5381;
  
  while ( *str )
  {
    hash = hash * 33 + (uint8_t)*str++;
  }
  
  return hash;
}

int main( int argc, const char* argv[] )
{
  int start, casestmt = 0, enumstmt = 0, prefix = 0, cpp = 0;
  char format[ 32 ];
  
  for ( start = 1; start < argc && argv[ start ][ 0 ] == '-' && argv[ start ][ 1 ] == '-'; start++ )
  {
    if ( !strcmp( argv[ start ], "--case" ) )
    {
      casestmt = 1;
    }
    else if ( !strcmp( argv[ start ], "--enum" ) )
    {
      enumstmt = 1;
    }
    else if ( !strcmp( argv[ start ], "--prefix" ) )
    {
      prefix = 1;
    }
    else if ( !strcmp( argv[ start ], "--cpp" ) )
    {
      cpp = 1;
    }
    else if ( !strcmp( argv[ start ], "--help" ) )
    {
      showhelp( stdout );
      return 0;
    }
    else
    {
      fprintf( stderr, "Unknown option: %s\n\n", argv[ start ] );
      showhelp( stderr );
      return 1;
    }
  }
  
  if ( enumstmt )
  {
    if ( casestmt )
    {
      if ( prefix )
      {
        strncpy( format, "case k%c%s:\n", sizeof( format ) );
      }
      else
      {
        strncpy( format, "case %s:\n", sizeof( format ) );
      }
    }
    else
    {
      int maxlen = 0, i;
      
      for ( i = start; i < argc; i++ )
      {
        int len = strlen( argv[ i ] );
        
        if ( len > maxlen )
        {
          maxlen = len;
        }
      }
      
      if ( prefix )
      {
        snprintf( format, sizeof( format ), "k%%c%%-%ds= 0x%%08xU,\n", maxlen );
      }
      else
      {
        snprintf( format, sizeof( format ), "%%-%ds = 0x%%08xU,\n", maxlen );
      }
    }
    
    format[ sizeof( format ) - 1 ] = 0;
  }
  
  for ( ; start < argc; start++ )
  {
    uint32_t hash = djb2( argv[ start ] );
    
    if ( casestmt )
    {
      if ( enumstmt )
      {
        if (prefix)
        {
          printf( format, toupper( argv[ start ][ 0 ] ), argv[ start ] + 1, hash );
        }
        else
        {
          printf( format, argv[ start ], hash );
        }
      }
      else
      {
        if ( cpp )
        {
          printf( "case 0x%08xU: // %s\n", hash, argv[ start ] );
        }
        else
        {
          printf( "case 0x%08xU: /* %s */\n", hash, argv[ start ] );
        }
      }
    }
    else if ( enumstmt )
    {
      if (prefix)
      {
        printf( format, toupper( argv[ start ][ 0 ] ), argv[ start ] + 1, hash );
      }
      else
      {
        printf( format, argv[ start ], hash );
      }
    }
    else
    {
      if ( cpp )
      {
        printf( "0x%08xU // %s\n", hash, argv[ start ] );
      }
      else
      {
        printf( "0x%08xU /* %s */\n", hash, argv[ start ] );
      }
    }
  }
  
  return 0;
}