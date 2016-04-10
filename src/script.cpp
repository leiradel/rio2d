/******************************************************************************
* Copyright (c) 2016 Andre Leiradella
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/

#include <setjmp.h>
#include <math.h>

#include "rio2d.h"

// Include easing functions taken from https://github.com/warrenm/AHEasing/blob/master/AHEasing/easing.c
#include "easing.inl"


rio2d::Hash rio2d::hash(const char* str)
{
  rio2d::Hash hash = 5381;

  if (*str != 0)
  {
    do
    {
      hash = hash * 33 + (uint8_t)*str++;
    } while (*str != 0);
  }

  return hash;
}

rio2d::Hash rio2d::hashLower(const char* str)
{
  rio2d::Hash hash = 5381;

  if (*str != 0)
  {
    do
    {
      hash = hash * 33 + (uint8_t)tolower(*str++);
    } while (*str != 0);
  }

  return hash;
}

rio2d::Hash rio2d::hash(const char* str, size_t length)
{
  rio2d::Hash hash = 5381;

  if (length != 0)
  {
    do
    {
      hash = hash * 33 + (uint8_t)*str++;
    } while (--length != 0);
  }

  return hash;
}

rio2d::Hash rio2d::hashLower(const char* str, size_t length)
{
  rio2d::Hash hash = 5381;

  if (length != 0)
  {
    do
    {
      hash = hash * 33 + (uint8_t)tolower(*str++);
    } while (--length != 0);
  }

  return hash;
}

namespace // Anonymous namespace to hyde the implementation details
{
  // Error codes.
  struct Errors
  {
    enum Enum
    {
      kOk,                      // No error
      kDuplicateIdentifier,     // Attempting to redefine an identifier
      kFirstParamNotANode,      // The first parameter of a subroutine must be a node
      kInvalidCharacterInInput, // Extraneous character in input
      kMalformedNumber,         // A number constant is malformed
      kOutOfMemory,             // Error allocating memory, or a fixed-size buffer was full
      kTypeMismatch,            // Wrong type in expression
      kUnexpectedEOF,           // The end of the file was reached
      kUnexpectedToken,         // This token wasn't expected here
      kUnknownEaseFunc,         // Unknown ease function
      kUnknownField,            // Invalid field for the object
      kUnknownIdentifier,       // Identifier not declared
      kUnknownType,             // Unknown data type
      kUnterminatedString,      // A string constant is unterminated
    };
  };

  // Tokens; the values of the symbols with only one character are their ASCII integer value.
  struct Tokens
  {
    enum
    {
      kAnd = 0x0b885e18U,
      kAs = 0x00597739U,
      kCeil = 0x7c9514a2U,
      kEnd = 0x0b886f1cU,
      kEof = 0,
      kFalse = 0x0f6bcef0U,
      kFloor = 0x0f71e367U,
      kForever = 0xbaa8bf3eU,
      kGreaterEqual = 1,
      kIdentifier = 2,
      kIn = 0x0059783cU,
      kLessEqual = 3,
      kLet = 0x0b888bcaU,
      kMod = 0x0b889145U,
      kMove = 0x7c9abc9cU,
      kNode = 0x7c9b46abU,
      kNot = 0x0b889596U,
      kNotEqual = 4,
      kNumber = 0x10f9208eU,
      kNumberConst = 5,
      kOr = 0x00597906U,
      kParallel = 0x1455e3f2U,
      kPause = 0x1020ea43U,
      kRand = 0x7c9d3deaU,
      kRem = 0x0b88a549U,
      kRepeat = 0x192dec66U,
      kSecs = 0x7c9dd9f3U,
      kSequence = 0x0c15489eU,
      kSignal = 0x1bc6ade3U,
      kSize = 0x7c9dede0U,
      kStringConst = 6,
      kSub = 0x0b88ab8fU,
      kTimes = 0x106d8b87U,
      kTrue = 0x7c9e9fe5U,
      kTrunc = 0x10729e11U,
      kVec2 = 0x7c9f7ed5U,
      kWith = 0x7ca01ea1U,
      kXor = 0x0b88c01eU,
    };
  };

  // Fields that can be addressed with objects.
  struct Fields
  {
    enum
    {
      // Hashes
      kBboxheight = 0xd92bfc69U,
      kBboxwidth = 0x7c035ed0U,
      kFadein = 0xfce10f4cU,
      kFadeout = 0x990313adU,
      kHeight = 0x01d688deU,
      kMoveby = 0x0e3c60b7U,
      kMoveto = 0x0e3c62ffU,
      kOpacity = 0x70951bfeU,
      kPosition = 0x4cef7abaU,
      kRotateby = 0x2737766fU,
      kRotateto = 0x273778b7U,
      kRotation = 0x27378915U,
      kScale = 0x1057f68dU,
      kScaleby = 0x862fdae8U,
      kScaleto = 0x862fdd30U,
      kVisible = 0x7c618d53U,
      kWidth = 0x10a3b0a5U,
      kX = 0x0002b61dU,
      kY = 0x0002b61eU,
      // Indices
      kBboxheightIndex = 0,
      kBboxwidthIndex = 1,
      kFadeinIndex = 2,
      kFadeoutIndex = 3,
      kHeightIndex = 4,
      kMovebyIndex = 5,
      kMovetoIndex = 6,
      kOpacityIndex = 7,
      kPositionIndex = 8,
      kRotatebyIndex = 9,
      kRotatetoIndex = 10,
      kRotationIndex = 11,
      kScaleIndex = 12,
      kScalebyIndex = 13,
      kScaletoIndex = 14,
      kVisibleIndex = 15,
      kWidthIndex = 16,
      kXIndex = 17,
      kYIndex = 18,
    };

    static inline rio2d::Script::Index index(rio2d::Hash hash)
    {
      switch (hash)
      {
      case kBboxheight:  return kBboxheightIndex;
      case kBboxwidth:   return kBboxwidthIndex;
      case kFadein:      return kFadeinIndex;
      case kFadeout:     return kFadeoutIndex;
      case kHeight:      return kHeightIndex;
      case kMoveby:      return kMovebyIndex;
      case kMoveto:      return kMovetoIndex;
      case kOpacity:     return kOpacityIndex;
      case kPosition:    return kPositionIndex;
      case kRotateby:    return kRotatebyIndex;
      case kRotateto:    return kRotatetoIndex;
      case kRotation:    return kRotationIndex;
      case kScale:       return kScaleIndex;
      case kScaleby:     return kScalebyIndex;
      case kScaleto:     return kScaletoIndex;
      case kVisible:     return kVisibleIndex;
      case kWidth:       return kWidthIndex;
      case kX:           return kXIndex;
      case kY:           return kYIndex;
      default:           return -1;
      }
    }
  };

  // Available easing functions.
  struct Easing
  {
    enum
    {
      // Hashes
      kBackin = 0xf38bf9edU,
      kBackinout = 0xd4b93685U,
      kBackout = 0x650b526eU,
      kBouncein = 0x66513df8U,
      kBounceinout = 0x32ae02b0U,
      kBounceout = 0x307917d9U,
      kCircin = 0xf679fe3dU,
      kCircinout = 0x1b4498d5U,
      kCircout = 0xc5b9e0beU,
      kCubicin = 0xe09955e2U,
      kCubicinout = 0xf5130a5aU,
      kCubicout = 0xf3c42d03U,
      kElasticin = 0xd27b95c1U,
      kElasticinout = 0x56bb31d9U,
      kElasticout = 0x21ee68c2U,
      kExpin = 0x0f6662e9U,
      kExpinout = 0xd3e4ce01U,
      kExpout = 0xfc32daeaU,
      kLinear = 0x0b7641e0U,
      kQuadin = 0x17f20ee7U,
      kQuadinout = 0x72dfe13fU,
      kQuadout = 0x163406a8U,
      kQuarticin = 0x944c3515U,
      kQuarticinout = 0xdde980adU,
      kQuarticout = 0x1dd2f296U,
      kQuinticin = 0xf2c97899U,
      kQuinticinout = 0x2c4c45b1U,
      kQuinticout = 0x4bf8a69aU,
      kSinein = 0x1bca5f4bU,
      kSineinout = 0x33cd0723U,
      kSineout = 0x9516638cU,
      // Indices
      kBackinIndex = 0,
      kBackinoutIndex = 1,
      kBackoutIndex = 2,
      kBounceinIndex = 3,
      kBounceinoutIndex = 4,
      kBounceoutIndex = 5,
      kCircinIndex = 6,
      kCircinoutIndex = 7,
      kCircoutIndex = 8,
      kCubicinIndex = 9,
      kCubicinoutIndex = 10,
      kCubicoutIndex = 11,
      kElasticinIndex = 12,
      kElasticinoutIndex = 13,
      kElasticoutIndex = 14,
      kExpinIndex = 15,
      kExpinoutIndex = 16,
      kExpoutIndex = 17,
      kLinearIndex = 18,
      kQuadinIndex = 19,
      kQuadinoutIndex = 20,
      kQuadoutIndex = 21,
      kQuarticinIndex = 22,
      kQuarticinoutIndex = 23,
      kQuarticoutIndex = 24,
      kQuinticinIndex = 25,
      kQuinticinoutIndex = 26,
      kQuinticoutIndex = 27,
      kSineinIndex = 28,
      kSineinoutIndex = 29,
      kSineoutIndex = 30,
    };

    static inline rio2d::Script::Index index(rio2d::Hash hash)
    {
      switch (hash)
      {
      case kBackin:        return kBackinIndex;
      case kBackinout:     return kBackinoutIndex;
      case kBackout:       return kBackoutIndex;
      case kBouncein:      return kBounceinIndex;
      case kBounceinout:   return kBounceinoutIndex;
      case kBounceout:     return kBounceoutIndex;
      case kCircin:        return kCircinIndex;
      case kCircinout:     return kCircinoutIndex;
      case kCircout:       return kCircoutIndex;
      case kCubicin:       return kCubicinIndex;
      case kCubicinout:    return kCubicinoutIndex;
      case kCubicout:      return kCubicoutIndex;
      case kElasticin:     return kElasticinIndex;
      case kElasticinout:  return kElasticinoutIndex;
      case kElasticout:    return kElasticoutIndex;
      case kExpin:         return kExpinIndex;
      case kExpinout:      return kExpinoutIndex;
      case kExpout:        return kExpoutIndex;
      case kLinear:        return kLinearIndex;
      case kQuadin:        return kQuadinIndex;
      case kQuadinout:     return kQuadinoutIndex;
      case kQuadout:       return kQuadoutIndex;
      case kQuarticin:     return kQuarticinIndex;
      case kQuarticinout:  return kQuarticinoutIndex;
      case kQuarticout:    return kQuarticoutIndex;
      case kQuinticin:     return kQuinticinIndex;
      case kQuinticinout:  return kQuinticinoutIndex;
      case kQuinticout:    return kQuinticoutIndex;
      case kSinein:        return kSineinIndex;
      case kSineinout:     return kSineinoutIndex;
      case kSineout:       return kSineoutIndex;
      default:             return -1;
      }
    }

    static inline rio2d::Script::Number evaluate(rio2d::Script::Index index, rio2d::Script::Number p)
    {
      typedef rio2d::Script::Number(*Ease)(rio2d::Script::Number);

      static const Ease functions[] =
      {
        BackEaseIn,
        BackEaseInOut,
        BackEaseOut,
        BounceEaseIn,
        BounceEaseInOut,
        BounceEaseOut,
        CircularEaseIn,
        CircularEaseInOut,
        CircularEaseOut,
        CubicEaseIn,
        CubicEaseInOut,
        CubicEaseOut,
        ElasticEaseIn,
        ElasticEaseInOut,
        ElasticEaseOut,
        ExponentialEaseIn,
        ExponentialEaseInOut,
        ExponentialEaseOut,
        LinearInterpolation,
        QuadraticEaseIn,
        QuadraticEaseInOut,
        QuadraticEaseOut,
        QuarticEaseIn,
        QuarticEaseInOut,
        QuarticEaseOut,
        QuinticEaseIn,
        QuinticEaseInOut,
        QuinticEaseOut,
        SineEaseIn,
        SineEaseInOut,
        SineEaseOut,
      };

      CCASSERT(index >= 0 && index < 31, "Invalid ease function index");
      return functions[index](p);
    }

  protected:
    static rio2d::Script::Number backin(rio2d::Script::Number p) { return p * p * p - p * sinf(p * M_PI); } // Modeled after the overshooting cubic y = x^3-x*sin(x*pi)
    static rio2d::Script::Number backinout(rio2d::Script::Number p) { return (p < 0.5f) ? 0.5f * ((p + p) * (p + p) * (p + p) - (p + p) * sinf((p + p) * M_PI)) : 0.5f * (1.0f - ((1.0f - (p + p - 1.0f)) * (1.0f - (p + p - 1.0f)) * (1.0f - (p + p - 1.0f)) - (1.0f - (p + p - 1.0f)) * sinf((1.0f - (p + p - 1.0f)) * M_PI))) + 0.5f; } // Modeled after the piecewise overshooting cubic function: || y = (1/2)*((2x)^3-(2x)*sin(2*x*pi)) ; [0, 0.5) || y = (1/2)*(1-((1-x)^3-(1-x)*sin((1-x)*pi))+1) ; [0.5, 1]
    static rio2d::Script::Number backout(rio2d::Script::Number p) { return 1.0f - ((1.0f - p) * (1.0f - p) * (1.0f - p) - (1.0f - p) * sinf((1.0f - p) * M_PI)); } // Modeled after overshooting cubic y = 1-((1-x)^3-(1-x)*sin((1-x)*pi))
    static rio2d::Script::Number bouncein(rio2d::Script::Number p) { return 1.0f - (((1.0f - p) < 4.0f / 11.0f) ? (121.0f * (1.0f - p) * (1.0f - p)) / 16.0f : ((1.0f - p) < 8.0f / 11.0f) ? (363.0f / 40.0f * (1.0f - p) * (1.0f - p)) - (99.0f / 10.0f * (1.0f - p)) + 17.0f / 5.0f : ((1.0f - p) < 9.0f / 10.0f) ? (4356.0f / 361.0f * (1.0f - p) * (1.0f - p)) - (35442.0f / 1805.0f * (1.0f - p)) + 16061.0f / 1805.0f : (54.0f / 5.0f * (1.0f - p) * (1.0f - p)) - (513.0f / 25.0f * (1.0f - p)) + 268.0f / 25.0f); }
    static rio2d::Script::Number bounceinout(rio2d::Script::Number p) { return (p < 0.5f) ? 0.5f * (1.0f - (((p + p) < 4.0f / 11.0f) ? (121.0f * (p + p) * (p + p)) / 16.0f : ((p + p) < 8.0f / 11.0f) ? (363.0f / 40.0f * (p + p) * (p + p)) - (99.0f / 10.0f * (p + p)) + 17.0f / 5.0f : ((p + p) < 9.0f / 10.0f) ? (4356.0f / 361.0f * (p + p) * (p + p)) - (35442.0f / 1805.0f * (p + p)) + 16061.0f / 1805.0f : (54.0f / 5.0f * (p + p) * (p + p)) - (513.0f / 25.0f * (p + p)) + 268.0f / 25.0f)) : 0.5f * (((p + p - 1.0f) < 4.0f / 11.0f) ? (121.0f * (p + p - 1.0f) * (p + p - 1.0f)) / 16.0f : ((p + p - 1.0f) < 8.0f / 11.0f) ? (363.0f / 40.0f * (p + p - 1.0f) * (p + p - 1.0f)) - (99.0f / 10.0f * (p + p - 1.0f)) + 17.0f / 5.0f : ((p + p - 1.0f) < 9.0f / 10.0f) ? (4356.0f / 361.0f * (p + p - 1.0f) * (p + p - 1.0f)) - (35442.0f / 1805.0f * (p + p - 1.0f)) + 16061.0f / 1805.0f : (54.0f / 5.0f * (p + p - 1.0f) * (p + p - 1.0f)) - (513.0f / 25.0f * (p + p - 1.0f)) + 268.0f / 25.0f) + 0.5f; }
    static rio2d::Script::Number bounceout(rio2d::Script::Number p) { return (p < 4.0f / 11.0f) ? (121.0f * p * p) / 16.0f : (p < 8.0f / 11.0f) ? (363.0f / 40.0f * p * p) - (99.0f / 10.0f * p) + 17.0f / 5.0f : (p < 9.0f / 10.0f) ? (4356.0f / 361.0f * p * p) - (35442.0f / 1805.0f * p) + 16061.0f / 1805.0f : (54.0f / 5.0f * p * p) - (513.0f / 25.0f * p) + 268.0f / 25.0f; }
    static rio2d::Script::Number circin(rio2d::Script::Number p) { return 1.0f - sqrtf(1.0f - (p * p)); } // Modeled after shifted quadrant IV of unit circle
    static rio2d::Script::Number circinout(rio2d::Script::Number p) { return (p < 0.5f) ? 0.5f * (1.0f - sqrtf(1.0f - 4.0f * (p * p))) : 0.5f * (sqrtf(-((2.0f * p) - 3.0f) * ((2.0f * p) - 1.0f)) + 1.0f); } // Modeled after the piecewise circular function || y = (1/2)(1 - sqrt(1 - 4x^2)) ; [0, 0.5) || y = (1/2)(sqrt(-(2x - 3)*(2x - 1)) + 1) ; [0.5, 1]
    static rio2d::Script::Number circout(rio2d::Script::Number p) { return sqrtf((2.0f - p) * p); } // Modeled after shifted quadrant II of unit circle
    static rio2d::Script::Number cubicin(rio2d::Script::Number p) { return p * p * p; } // Modeled after the cubic y = x^3
    static rio2d::Script::Number cubicinout(rio2d::Script::Number p) { return (p < 0.5f) ? 4.0f * p * p * p : 0.5f * ((2.0f * p) - 2.0f) * ((2.0f * p) - 2.0f) * ((2.0f * p) - 2.0f) + 1.0f; } // Modeled after the piecewise cubic || y = (1/2)((2x)^3) ; [0, 0.5) || y = (1/2)((2x-2)^3 + 2) ; [0.5, 1]
    static rio2d::Script::Number cubicout(rio2d::Script::Number p) { return (p - 1.0f) * (p - 1.0f) * (p - 1.0f) + 1.0f; } // Modeled after the cubic y = (x - 1)^3 + 1
    static rio2d::Script::Number elasticin(rio2d::Script::Number p) { return sinf(13.0f * M_PI_2 * p) * powf(2.0f, 10.0f * (p - 1.0f)); } // Modeled after the damped sine wave y = sin(13pi/2*x)*pow(2, 10 * (x - 1))
    static rio2d::Script::Number elasticinout(rio2d::Script::Number p) { return (p < 0.5f) ? 0.5f * sinf(13.0f * M_PI_2 * (2.0f * p)) * powf(2.0f, 10.0f * ((2.0f * p) - 1.0f)) : 0.5f * (sinf(-13.0f * M_PI_2 * ((2.0f * p - 1.0f) + 1.0f)) * powf(2.0f, -10.0f * (2.0f * p - 1.0f)) + 2.0f); } // Modeled after the piecewise exponentially-damped sine wave: || y = (1/2)*sin(13pi/2*(2*x))*pow(2, 10 * ((2*x) - 1)) ; [0,0.5) || y = (1/2)*(sin(-13pi/2*((2x-1)+1))*pow(2,-10(2*x-1)) + 2) ; [0.5, 1]
    static rio2d::Script::Number elasticout(rio2d::Script::Number p) { return sinf(-13.0f * M_PI_2 * (p + 1.0f)) * powf(2.0f, -10.0f * p) + 1.0f; } // Modeled after the damped sine wave y = sin(-13pi/2*(x + 1))*pow(2, -10x) + 1
    static rio2d::Script::Number expin(rio2d::Script::Number p) { return (p == 0.0f) ? p : powf(2.0f, 10.0f * (p - 1.0f)); } // Modeled after the exponential function y = 2^(10(x - 1))
    static rio2d::Script::Number expinout(rio2d::Script::Number p) { return (p == 0.0f || p == 1.0f) ? p : (p < 0.5f) ? 0.5f * powf(2.0f, (20.0f * p) - 10.0f) : -0.5f * powf(2.0f, (-20.0f * p) + 10.0f) + 1.0f; } // Modeled after the piecewise exponential || y = (1/2)2^(10(2x - 1)); [0,0.5) || y = -(1/2)*2^(-10(2x - 1))) + 1 ; [0.5,1]
    static rio2d::Script::Number expout(rio2d::Script::Number p) { return (p == 1.0f) ? p : 1.0f - powf(2.0f, -10.0f * p); } // Modeled after the exponential function y = -2^(-10x) + 1
    static rio2d::Script::Number linear(rio2d::Script::Number p) { return p; } // Modeled after the line y = x
    static rio2d::Script::Number quadin(rio2d::Script::Number p) { return p * p; } // Modeled after the parabola y = x^2
    static rio2d::Script::Number quadinout(rio2d::Script::Number p) { return (p < 0.5f) ? 2.0f * p * p : (-2.0f * p * p) + (4.0f * p) - 1.0f; } // Modeled after the piecewise quadratic || y = (1/2)((2x)^2) ; [0, 0.5) || y = -(1/2)((2x-1)*(2x-3) - 1) ; [0.5, 1]
    static rio2d::Script::Number quadout(rio2d::Script::Number p) { return -(p * (p - 2.0f)); } // Modeled after the parabola y = -x^2 + 2x
    static rio2d::Script::Number quarticin(rio2d::Script::Number p) { return p * p * p * p; } // Modeled after the quartic x^4
    static rio2d::Script::Number quarticinout(rio2d::Script::Number p) { return (p < 0.5f) ? 8.0f * p * p * p * p : -8.0f * (p - 1.0f) * (p - 1.0f) * (p - 1.0f) * (p - 1.0f) + 1.0f; } // Modeled after the piecewise quartic || y = (1/2)((2x)^4) ; [0, 0.5) || y = -(1/2)((2x-2)^4 - 2) ; [0.5, 1]
    static rio2d::Script::Number quarticout(rio2d::Script::Number p) { return (p - 1.0f) * (p - 1.0f) * (p - 1.0f) * (1.0f - p) + 1.0f; } // Modeled after the quartic y = 1 - (x - 1)^4
    static rio2d::Script::Number quinticin(rio2d::Script::Number p) { return p * p * p * p * p; } // Modeled after the quintic y = x^5
    static rio2d::Script::Number quinticinout(rio2d::Script::Number p) { return (p < 0.5f) ? 16.0f * p * p * p * p * p : 0.5f * ((p + p) - 2.0f) * ((p + p) - 2.0f) * ((p + p) - 2.0f) * ((p + p) - 2.0f) * ((p + p) - 2.0f) + 1.0f; } // Modeled after the piecewise quintic || y = (1/2)((2x)^5) ; [0, 0.5) || y = (1/2)((2x-2)^5 + 2) ; [0.5, 1]
    static rio2d::Script::Number quinticout(rio2d::Script::Number p) { return (p - 1.0f) * (p - 1.0f) * (p - 1.0f) * (p - 1.0f) * (p - 1.0f) + 1.0f; } // Modeled after the quintic y = (x - 1)^5 + 1
    static rio2d::Script::Number sinein(rio2d::Script::Number p) { return sinf((p - 1.0f) * M_PI_2) + 1.0f; } // Modeled after quarter-cycle of sine wave
    static rio2d::Script::Number sineinout(rio2d::Script::Number p) { return 0.5f * (1.0f - cosf(p * M_PI)); } // Modeled after half sine wave
    static rio2d::Script::Number sineout(rio2d::Script::Number p) { return sinf(p * M_PI_2); } // Modeled after quarter-cycle of sine wave (different phase)
  };

  // Bytecode instructions.
  struct Insns
  {
    enum
    {
      kAdd = 0,
      kCallMethod = 1,
      kCeil = 2,
      kCmpEqual = 3,
      kCmpGreater = 4,
      kCmpGreaterEqual = 5,
      kCmpLess = 6,
      kCmpLessEqual = 7,
      kCmpNotEqual = 8,
      kDiv = 9,
      kDrop = 10,
      kDup = 11,
      kFloor = 12,
      kGetGlobal = 13,
      kGetProp = 14,
      kJnz = 15,
      kJump = 16,
      kLogicalAnd = 17,
      kLogicalNot = 18,
      kLogicalOr = 19,
      kModulus = 20,
      kMul = 21,
      kNeg = 22,
      kPause = 23,
      kPush = 24,
      kRand = 25,
      kRandRange = 26,
      kSetGlobal = 27,
      kSetProp = 28,
      kSignal = 29,
      kSpawn = 30,
      kStop = 31,
      kSub = 32,
      kTrunc = 33,
      kVaryAbs = 34,
      kVaryRel = 35,
    };

    static inline size_t size(rio2d::Script::Insn insn)
    {
      static const uint8_t sizes[] =
      {
        1, // kAdd
        3, // kCallMethod
        1, // kCeil
        1, // kCmpEqual
        1, // kCmpGreater
        1, // kCmpGreaterEqual
        1, // kCmpLess
        1, // kCmpLessEqual
        1, // kCmpNotEqual
        1, // kDiv
        1, // kDrop
        1, // kDup
        1, // kFloor
        2, // kGetGlobal
        3, // kGetProp
        2, // kJnz
        2, // kJump
        1, // kLogicalAnd
        1, // kLogicalNot
        1, // kLogicalOr
        1, // kModulus
        1, // kMul
        1, // kNeg
        1, // kPause
        2, // kPush
        1, // kRand
        1, // kRandRange
        2, // kSetGlobal
        3, // kSetProp
        2, // kSignal
        2, // kSpawn
        1, // kStop
        1, // kSub
        1, // kTrunc
        4, // kVaryAbs
        4, // kVaryRel
      };

      CCASSERT(insn >= 0 && insn < 36, "Invalid instruction");
      return sizes[insn];
    };

#ifndef NDEBUG
    static void disasm(rio2d::Script::Address addr, const rio2d::Script::Bytecode*& bc, const char* prefix = "")
    {
      switch (bc->m_insn)
      {
      case kAdd:             CCLOG("%s%04x\t%08x\tadd", prefix, addr, bc->m_insn); bc += 1; break;
      case kCallMethod:      CCLOG("%s%04x\t%08x\tcall_method l@%d f@%d", prefix, addr, bc->m_insn, bc[1].m_index, bc[2].m_index); bc += 3; break;
      case kCeil:            CCLOG("%s%04x\t%08x\tceil", prefix, addr, bc->m_insn); bc += 1; break;
      case kCmpEqual:        CCLOG("%s%04x\t%08x\tcmp_eq", prefix, addr, bc->m_insn); bc += 1; break;
      case kCmpGreater:      CCLOG("%s%04x\t%08x\tcmp_gt", prefix, addr, bc->m_insn); bc += 1; break;
      case kCmpGreaterEqual: CCLOG("%s%04x\t%08x\tcmp_ge", prefix, addr, bc->m_insn); bc += 1; break;
      case kCmpLess:         CCLOG("%s%04x\t%08x\tcmp_lt", prefix, addr, bc->m_insn); bc += 1; break;
      case kCmpLessEqual:    CCLOG("%s%04x\t%08x\tcmp_le", prefix, addr, bc->m_insn); bc += 1; break;
      case kCmpNotEqual:     CCLOG("%s%04x\t%08x\tcmp_ne", prefix, addr, bc->m_insn); bc += 1; break;
      case kDiv:             CCLOG("%s%04x\t%08x\tdiv", prefix, addr, bc->m_insn); bc += 1; break;
      case kDrop:            CCLOG("%s%04x\t%08x\tdrop", prefix, addr, bc->m_insn); bc += 1; break;
      case kDup:             CCLOG("%s%04x\t%08x\tdup", prefix, addr, bc->m_insn); bc += 1; break;
      case kFloor:           CCLOG("%s%04x\t%08x\tfloor", prefix, addr, bc->m_insn); bc += 1; break;
      case kGetGlobal:       CCLOG("%s%04x\t%08x\tget_global l@%d", prefix, addr, bc->m_insn, bc[1].m_index); bc += 2; break;
      case kGetProp:         CCLOG("%s%04x\t%08x\tget_field l@%d f@%d", prefix, addr, bc->m_insn, bc[1].m_index, bc[2].m_index); bc += 3; break;
      case kJnz:             CCLOG("%s%04x\t%08x\tjnz %04x", prefix, addr, bc->m_insn, bc[1].m_address); bc += 2; break;
      case kJump:            CCLOG("%s%04x\t%08x\tjump %04x", prefix, addr, bc->m_insn, bc[1].m_address); bc += 2; break;
      case kLogicalAnd:      CCLOG("%s%04x\t%08x\tand", prefix, addr, bc->m_insn); bc += 1; break;
      case kLogicalNot:      CCLOG("%s%04x\t%08x\tnot", prefix, addr, bc->m_insn); bc += 1; break;
      case kLogicalOr:       CCLOG("%s%04x\t%08x\tor", prefix, addr, bc->m_insn); bc += 1; break;
      case kModulus:         CCLOG("%s%04x\t%08x\tmod", prefix, addr, bc->m_insn); bc += 1; break;
      case kMul:             CCLOG("%s%04x\t%08x\tmul", prefix, addr, bc->m_insn); bc += 1; break;
      case kNeg:             CCLOG("%s%04x\t%08x\tneg", prefix, addr, bc->m_insn); bc += 1; break;
      case kPause:           CCLOG("%s%04x\t%08x\tpause", prefix, addr, bc->m_insn); bc += 1; break;
      case kPush:            CCLOG("%s%04x\t%08x\tpush %f", prefix, addr, bc->m_insn, bc[1].m_number); bc += 2; break;
      case kRand:            CCLOG("%s%04x\t%08x\trand", prefix, addr, bc->m_insn); bc += 1; break;
      case kRandRange:       CCLOG("%s%04x\t%08x\trandr", prefix, addr, bc->m_insn); bc += 1; break;
      case kSetGlobal:       CCLOG("%s%04x\t%08x\tset_global l@%d", prefix, addr, bc->m_insn, bc[1].m_index); bc += 2; break;
      case kSetProp:         CCLOG("%s%04x\t%08x\tset_field l@%d f@%d", prefix, addr, bc->m_insn, bc[1].m_index, bc[2].m_index); bc += 3; break;
      case kSignal:          CCLOG("%s%04x\t%08x\tsignal #%08x", prefix, addr, bc->m_insn, bc[1]); bc += 2; break;
      case kSpawn:           CCLOG("%s%04x\t%08x\tspawn %04x", prefix, addr, bc->m_insn, bc[1]); bc += 2; break;
      case kStop:            CCLOG("%s%04x\t%08x\tstop", prefix, addr, bc->m_insn); bc += 1; break;
      case kSub:             CCLOG("%s%04x\t%08x\tsub", prefix, addr, bc->m_insn); bc += 1; break;
      case kTrunc:           CCLOG("%s%04x\t%08x\ttrunc", prefix, addr, bc->m_insn); bc += 1; break;
      case kVaryAbs:         CCLOG("%s%04x\t%08x\tvary_abs l@%d f@%d e@%d", prefix, addr, bc->m_insn, bc[1].m_index, bc[2].m_index, bc[3].m_index); bc += 4; break;
      case kVaryRel:         CCLOG("%s%04x\t%08x\tvary_rel l@%d f@%d e@%d", prefix, addr, bc->m_insn, bc[1].m_index, bc[2].m_index, bc[3].m_index); bc += 4; break;
      default:               CCLOG("%s%04x	%08x	unknown", prefix, addr, bc->m_insn); bc += 1; break;
      }
    }

    static void disasm(const rio2d::Script::Bytecode* bc, const rio2d::Script::Bytecode* end)
    {
      const rio2d::Script::Bytecode* start = bc;

      while (bc < end)
      {
        disasm(bc - start, bc);
      }
    }
#endif
  };

  class Emitter
  {
  public:
    virtual Errors::Enum addGlobal(rio2d::Hash hash) = 0;
    virtual size_t       numGlobals() const = 0;

    virtual Errors::Enum addLocal(rio2d::Hash hash, rio2d::Script::Token type) = 0;
    virtual size_t       numLocals() const = 0;
    virtual Errors::Enum getIndex(rio2d::Hash hash, rio2d::Script::Index* index) const = 0;
    virtual Errors::Enum getType(rio2d::Hash hash, rio2d::Script::Token* type) const = 0;

    virtual void                   emit(rio2d::Script::Insn insn, va_list args) = 0;
    virtual rio2d::Script::Address getPC() const = 0;
    virtual void                   patch(rio2d::Script::Address address, rio2d::Script::Bytecode bc) = 0;
  };

  // This emitter just collects info about the script without generating any code.
  class CounterEmitter : public Emitter
  {
  protected:
    struct Local
    {
      rio2d::Hash  m_hash;
      rio2d::Script::Token m_type;
    };

    rio2d::Hash m_globals[rio2d::Script::kMaxGlobals];
    Local  m_locals[rio2d::Script::kMaxLocalVars];
    size_t m_numGlobals;
    size_t m_numLocals;
    rio2d::Script::Address m_pc;

  public:
    inline void init()
    {
      m_pc = 0;
      m_numGlobals = 0;
    }

    Errors::Enum addGlobal(rio2d::Hash hash)
    {
      if (m_numGlobals < rio2d::Script::kMaxGlobals)
      {
        rio2d::Hash* global = m_globals;
        const rio2d::Hash* end = global + m_numGlobals;

        while (global < end)
        {
          if (*m_globals == hash)
          {
            return Errors::kDuplicateIdentifier;
          }

          global++;
        }

        *global = hash;
        m_numGlobals++;
        m_numLocals = 0;
        return Errors::kOk;
      }

      return Errors::kOutOfMemory;
    }

    size_t numGlobals() const
    {
      return m_numGlobals;
    }

    Errors::Enum addLocal(rio2d::Hash hash, rio2d::Script::Token type)
    {
      if (m_numLocals < rio2d::Script::kMaxLocalVars)
      {
        Local* local = m_locals;
        const Local* end = local + m_numLocals;

        while (local < end)
        {
          if (local->m_hash == hash)
          {
            if (local->m_type == type)
            {
              return Errors::kOk;
            }

            return Errors::kTypeMismatch;
          }

          local++;
        }

        local->m_hash = hash;
        local->m_type = type;
        m_numLocals++;
        return Errors::kOk;
      }

      return Errors::kOutOfMemory;
    }

    size_t numLocals() const
    {
      return m_numLocals;
    }

    Errors::Enum getIndex(rio2d::Hash hash, rio2d::Script::Index* index) const
    {
      const Local* local = m_locals;
      const Local* end = local + m_numLocals;

      while (local < end)
      {
        if (local->m_hash == hash)
        {
          *index = local - m_locals;
          return Errors::kOk;
        }

        local++;
      }

      return Errors::kUnknownIdentifier;
    }

    Errors::Enum getType(rio2d::Hash hash, rio2d::Script::Token* type) const
    {
      const Local* local = m_locals;
      const Local* end = local + m_numLocals;

      while (local < end)
      {
        if (local->m_hash == hash)
        {
          *type = local->m_type;
          return Errors::kOk;
        }

        local++;
      }

      return Errors::kUnknownIdentifier;
    }

    virtual void emit(rio2d::Script::Insn insn, va_list args)
    {
      m_pc += Insns::size(insn);
    }

    virtual rio2d::Script::Address getPC() const
    {
      return m_pc;
    }

    virtual void patch(rio2d::Script::Address address, rio2d::Script::Bytecode bc)
    {
      (void)address;
      (void)bc;
    }
  };

  class CodeEmitter : public Emitter
  {
  protected:
    rio2d::Script::Bytecode* m_bytecode;
    rio2d::Script::Address m_pc;

    rio2d::Script::Subroutine* m_globals;
    size_t m_numGlobals;

  public:
    inline void initWithMemory(rio2d::Script::Bytecode* bytecode, rio2d::Script::Subroutine* globals)
    {
      m_globals = globals;
      m_numGlobals = 0;

      m_bytecode = bytecode;
      m_pc = 0;
    }

    Errors::Enum addGlobal(rio2d::Hash hash)
    {
      rio2d::Script::Subroutine* global = m_globals + m_numGlobals++;

      global->m_hash = hash;
      global->m_pc = m_pc;
      global->m_numLocals = 0;

      return Errors::kOk;
    }

    size_t numGlobals() const
    {
      return m_numGlobals;
    }

    Errors::Enum addLocal(rio2d::Hash hash, rio2d::Script::Token type)
    {
      if (m_numGlobals != 0)
      {
        rio2d::Script::Subroutine* global = m_globals + m_numGlobals - 1;
        rio2d::Script::LocalVar* local = global->m_locals;
        const rio2d::Script::LocalVar* end = local + global->m_numLocals;

        while (local < end)
        {
          if (local->m_hash == hash)
          {
            if (local->m_type == type)
            {
              return Errors::kOk;
            }

            return Errors::kTypeMismatch;
          }

          local++;
        }

        local->m_hash = hash;
        local->m_type = type;
        global->m_numLocals++;
      }

      return Errors::kOk;
    }

    virtual size_t numLocals() const
    {
      if (m_numGlobals != 0)
      {
        const rio2d::Script::Subroutine* global = m_globals + m_numGlobals - 1;
        return global->m_numLocals;
      }

      return 0;
    }

    Errors::Enum getIndex(rio2d::Hash hash, rio2d::Script::Index* index) const
    {
      if (m_numGlobals != 0)
      {
        const rio2d::Script::Subroutine* global = m_globals + m_numGlobals - 1;
        const rio2d::Script::LocalVar* local = global->m_locals;
        const rio2d::Script::LocalVar* end = local + global->m_numLocals;

        while (local < end)
        {
          if (local->m_hash == hash)
          {
            *index = local - global->m_locals;
            return Errors::kOk;
          }

          local++;
        }
      }

      return Errors::kUnknownIdentifier;
    }

    Errors::Enum getType(rio2d::Hash hash, rio2d::Script::Token* type) const
    {
      if (m_numGlobals != 0)
      {
        const rio2d::Script::Subroutine* global = m_globals + m_numGlobals - 1;
        const rio2d::Script::LocalVar* local = global->m_locals;
        const rio2d::Script::LocalVar* end = local + global->m_numLocals;

        while (local < end)
        {
          if (local->m_hash == hash)
          {
            *type = local->m_type;
            return Errors::kOk;
          }

          local++;
        }
      }

      return Errors::kUnknownIdentifier;
    }

    virtual void emit(rio2d::Script::Insn insn, va_list args)
    {
      switch (insn)
      {
      case Insns::kAdd:
      case Insns::kCeil:
      case Insns::kCmpEqual:
      case Insns::kCmpGreater:
      case Insns::kCmpGreaterEqual:
      case Insns::kCmpLess:
      case Insns::kCmpLessEqual:
      case Insns::kCmpNotEqual:
      case Insns::kDiv:
      case Insns::kDrop:
      case Insns::kDup:
      case Insns::kFloor:
      case Insns::kLogicalAnd:
      case Insns::kLogicalNot:
      case Insns::kLogicalOr:
      case Insns::kModulus:
      case Insns::kMul:
      case Insns::kNeg:
      case Insns::kPause:
      case Insns::kRand:
      case Insns::kRandRange:
      case Insns::kStop:
      case Insns::kSub:
      case Insns::kTrunc:
        m_bytecode[m_pc++].m_insn = insn;
        break;

      case Insns::kJnz:
      case Insns::kJump:
      case Insns::kSpawn:
        m_bytecode[m_pc++].m_insn = insn;
        m_bytecode[m_pc++].m_address = va_arg(args, rio2d::Script::Address);
        break;

      case Insns::kSignal:
        m_bytecode[m_pc++].m_insn = insn;
        m_bytecode[m_pc++].m_hash = va_arg(args, rio2d::Hash);
        break;

      case Insns::kPush:
        m_bytecode[m_pc++].m_insn = insn;
        m_bytecode[m_pc++].m_number = (rio2d::Script::Number)va_arg(args, double);
        break;

      case Insns::kGetGlobal:
      case Insns::kSetGlobal:
        m_bytecode[m_pc++].m_insn = insn;
        m_bytecode[m_pc++].m_index = va_arg(args, rio2d::Script::Index);
        break;

      case Insns::kCallMethod:
      case Insns::kGetProp:
      case Insns::kSetProp:
        m_bytecode[m_pc++].m_insn = insn;
        m_bytecode[m_pc++].m_index = va_arg(args, rio2d::Script::Index);
        m_bytecode[m_pc++].m_index = va_arg(args, rio2d::Script::Index);
        break;

      case Insns::kVaryAbs:
      case Insns::kVaryRel:
        m_bytecode[m_pc++].m_insn = insn;
        m_bytecode[m_pc++].m_index = va_arg(args, rio2d::Script::Index);
        m_bytecode[m_pc++].m_index = va_arg(args, rio2d::Script::Index);
        m_bytecode[m_pc++].m_index = va_arg(args, rio2d::Script::Index);
        break;
      }
    }

    virtual uint32_t getPC() const
    {
      return m_pc;
    }

    virtual void patch(rio2d::Script::Address address, rio2d::Script::Bytecode bc)
    {
      CCASSERT(address < m_pc, "Invalid address to patch");
      m_bytecode[address] = bc;
    }
  };

  class Parser
  {
  protected:
    const char* m_current;
    unsigned m_line;

    const char* m_lexeme;
    size_t m_length;
    rio2d::Hash m_hash;
    rio2d::Script::Token m_token;
    unsigned m_tokenLine;

    // On Windows it seems exception handling on x86_64 is fine, but it isn't on x86 and
    // I don't know about other CPUs/architectures. Let's avoid the overhead.
    jmp_buf m_rollback;

    rio2d::Script::Index m_globalsIndex;
    rio2d::Script::Bytecode* m_bytecode;
    rio2d::Script::Subroutine* m_globals;

    Emitter* m_emitter;

#ifndef NDEBUG
    size_t m_bcSize;
    size_t m_numGlobals;
#endif

  public:
    Errors::Enum initWithSourceAndPointers(const char* source, rio2d::Script::Bytecode** bytecode, size_t* bcSize, rio2d::Script::Subroutine** globals, size_t* numGlobals)
    {
      CounterEmitter counter;
      counter.init();
      m_emitter = &counter;

      Errors::Enum res = compile(source);

      if (res != Errors::kOk)
      {
        return res;
      }

#ifndef NDEBUG
      m_bcSize = *bcSize = (size_t)counter.getPC();
      m_numGlobals = *numGlobals = counter.numGlobals();
#else
      *bcSize = (size_t)counter.getPC();
      *numGlobals = counter.numGlobals();
#endif

      m_bytecode = new rio2d::Script::Bytecode[*bcSize];
      m_globals = new rio2d::Script::Subroutine[*numGlobals];

      if (m_bytecode != nullptr && m_globals != nullptr)
      {
        *bytecode = m_bytecode;
        *globals = m_globals;

        CodeEmitter generator;
        generator.initWithMemory(m_bytecode, m_globals);
        m_emitter = &generator;

        res = compile(source); // This call to compile is bound to return kOk.

#ifndef NDEBUG
        Insns::disasm(m_bytecode, m_bytecode + *bcSize);
#endif

        return res;
      }

      if (m_bytecode != nullptr)
      {
        delete[] m_bytecode;
      }

      if (m_globals != nullptr)
      {
        delete[] m_globals;
      }

      return Errors::kOutOfMemory;
    }

    unsigned getLine() const
    {
      return m_tokenLine;
    }

    const char* getLexeme(size_t* length)
    {
      if (length)
      {
        *length = m_length;
      }

      return m_lexeme;
    }

  protected:
    int raise(Errors::Enum error)
    {
      longjmp(m_rollback, (int)error);
      return 0; // Shut up the compiler.
    }

    Errors::Enum compile(const char* source)
    {
      m_current = source;
      m_line = 1;

      Errors::Enum res = (Errors::Enum)setjmp(m_rollback);

      if (res != Errors::kOk)
      {
        goto out;
      }

      m_globalsIndex = 0;

      match();
      parse();

      res = Errors::kOk;

    out:
      return res;
    }

    void match()
    {
    again:

      // Skip spaces.
      for (;;)
      {
        if (isspace(*m_current))
        {
          if (*m_current == '\n')
          {
            // End of line, increment the line number.
            m_line++;
          }

          m_current++; // m_current is a space, can increment directly.
        }
        else if (*m_current != 0)
        {
          // Not a space, and not the end of the source code.
          break;
        }
        else
        {
          // End of the source code, return the kEof token.
          m_lexeme = "<eof>";
          m_length = 5;
          m_token = Tokens::kEof;
          m_tokenLine = m_line;
          return;
        }
      }

      m_lexeme = m_current;
      m_tokenLine = m_line;

      // If the character is alphabetic or '_', the token is a keyword or an identifier.
      if (isalpha(*m_current) || *m_current == '_')
      {
        do
        {
          m_current++; // m_current is an alphanumeric character, can increment directly.
        } while (isalnum(*m_current) || *m_current == '_');

        // Evaluate the hash of the identifier to check if it's a keyword.
        m_hash = rio2d::hashLower(m_lexeme, m_current - m_lexeme);

        switch (m_hash)
        {
        case Tokens::kAnd:
        case Tokens::kAs:
        case Tokens::kCeil:
        case Tokens::kEnd:
        case Tokens::kFalse:
        case Tokens::kFloor:
        case Tokens::kForever:
        case Tokens::kIn:
        case Tokens::kLet:
        case Tokens::kMod:
        case Tokens::kMove:
        case Tokens::kNode:
        case Tokens::kNot:
        case Tokens::kNumber:
        case Tokens::kOr:
        case Tokens::kParallel:
        case Tokens::kPause:
        case Tokens::kRand:
        case Tokens::kRepeat:
        case Tokens::kSecs:
        case Tokens::kSequence:
        case Tokens::kSignal:
        case Tokens::kSize:
        case Tokens::kSub:
        case Tokens::kTimes:
        case Tokens::kTrue:
        case Tokens::kTrunc:
        case Tokens::kVec2:
        case Tokens::kWith:
        case Tokens::kXor:
          m_token = m_hash;
          break;

        case Tokens::kRem: // Comment.
          while (*m_current != '\n' && *m_current != 0) m_current++;

          // Could be return next(...) since compilers do tail call optimization blah blah blah,
          // but the optimization would be just a jump anyway, just like this goto here :P
          goto again;

        default:
          m_token = Tokens::kIdentifier;
          break;
        }

        m_length = m_current - m_lexeme;
        return;
      }

      if (isdigit(*m_current))
      {
        do
        {
          m_current++; // m_current is a decimal digit, can increment directly.
        } while (isdigit(*m_current));

        if (*m_current == '.')
        {
          m_current++; // m_current is '.', can increment directly.

          if (!isdigit(*m_current))
          {
            raise(Errors::kMalformedNumber);
            return; // Not needed, but let's the compiler know we're not going to do any further processing here.
          }

          do
          {
            m_current++; // m_current is a decimal digit, can increment directly.
          } while (isdigit(*m_current));
        }

        if (*m_current == 'e' || *m_current == 'E')
        {
          m_current++; // m_current is the exponent character, can increment directly.

          if (*m_current == '-' || *m_current == '+')
          {
            m_current++; // m_current is a signal, can increment directly.
          }

          if (!isdigit(*m_current))
          {
            raise(Errors::kMalformedNumber);
            return;
          }

          do
          {
            m_current++; // m_current is a decimal digit, can increment directly.
          } while (isdigit(*m_current));
        }

        m_length = m_current - m_lexeme;
        m_token = Tokens::kNumberConst;
        return;
      }

      if (*m_current == '"')
      {
        m_current++; // m_current is '"', can increment directly.

        while (*m_current != 0 && *m_current != '\n')
        {
          if (*m_current == '"')
          {
            if (m_current[1] == '"')
            {
              m_current++;
            }
            else
            {
              break;
            }
          }

          m_current++; // m_current cannot be the null terminator, can increment directly.
        }

        if (*m_current != '"')
        {
          raise(Errors::kUnterminatedString);
          return;
        }

        m_lexeme++;
        m_length = m_current++ - m_lexeme; // m_current is '"', can increment directly.
        m_token = Tokens::kStringConst;
        m_hash = rio2d::hash(m_lexeme, m_length);
        return;
      }

      switch (*m_current)
      {
      case '+':
      case '-':
      case '*':
      case '/':
      case '=':
      case '.':
      case ',':
      case '(':
      case ')':
        m_length = 1;
        m_token = *m_current++; // m_current cannot be the null terminator, can increment directly.
        return;

      case '<':
        m_token = *m_current++; // m_current cannot be the null terminator, can increment directly.

        if (*m_current == '>')
        {
          m_current++; // m_current cannot be the null terminator, can increment directly.
          m_token = Tokens::kNotEqual;
        }
        else if (*m_current == '=')
        {
          m_current++; // m_current cannot be the null terminator, can increment directly.
          m_token = Tokens::kLessEqual;
        }

        m_length = m_current - m_lexeme;
        return;

      case '>':
        m_token = *m_current++; // m_current cannot be the null terminator, can increment directly.

        if (*m_current == '=')
        {
          m_current++; // m_current cannot be the null terminator, can increment directly.
          m_token = Tokens::kGreaterEqual;
        }

        m_length = m_current - m_lexeme;
        return;
      }

      if (*m_current == 0)
      {
        raise(Errors::kUnexpectedEOF);
        return;
      }

      raise(Errors::kInvalidCharacterInInput);
    }

    void match(rio2d::Script::Token token)
    {
      if (m_token != token)
      {
        raise(Errors::kUnexpectedToken);
        return;
      }

      match();
    }

    void emit(rio2d::Script::Insn insn, ...)
    {
      va_list args;
      va_start(args, insn);

      m_emitter->emit(insn, args);

      va_end(args);
    }

    void emitNodeVary(bool absolute, rio2d::Script::Index index)
    {
      rio2d::Script::Index field = Fields::index(m_hash);
      unsigned params;

      switch (field)
      {
      case Fields::kFadeinIndex:
        field = Fields::kOpacityIndex;
        params = 0;
        emit(Insns::kPush, 0.0f);
        emit(Insns::kPush, 255.0f);
        break;

      case Fields::kFadeoutIndex:
        field = Fields::kOpacityIndex;
        params = 0;
        emit(Insns::kPush, 255.0f);
        emit(Insns::kPush, 0.0f);
        break;

      case Fields::kRotatebyIndex:
      case Fields::kRotatetoIndex:
        field = Fields::kRotationIndex;
        params = 1;
        emit(Insns::kGetProp, index, Fields::kRotationIndex);
        break;

      case Fields::kScalebyIndex:
      case Fields::kScaletoIndex:
        field = Fields::kScaleIndex;
        params = 1;
        emit(Insns::kGetProp, index, Fields::kScaleIndex);
        break;

      case Fields::kMovebyIndex:
      case Fields::kMovetoIndex:
        field = Fields::kPositionIndex;
        params = 2;
        emit(Insns::kGetProp, index, Fields::kXIndex);
        emit(Insns::kGetProp, index, Fields::kYIndex);
        break;
      }

      match(Tokens::kIdentifier);

      if (params-- != 0)
      {
        if (parseExpression() != Tokens::kNumber)
        {
          raise(Errors::kTypeMismatch);
          return;
        }

        while (params-- != 0)
        {
          match(',');

          if (parseExpression() != Tokens::kNumber)
          {
            raise(Errors::kTypeMismatch);
            return;
          }
        }
      }

      match(Tokens::kIn);

      // Push the elapsed time.
      emit(Insns::kPush, 0.0f);

      if (parseExpression() != Tokens::kNumber)
      {
        raise(Errors::kTypeMismatch);
        return;
      }

      match(Tokens::kSecs);

      rio2d::Script::Index ease = Easing::kLinearIndex;

      if (m_token == Tokens::kWith)
      {
        match();
        ease = Easing::index(m_hash);

        if (ease == -1)
        {
          raise(Errors::kUnknownEaseFunc);
        }

        match(Tokens::kIdentifier);
      }

      emit(absolute ? Insns::kVaryAbs : Insns::kVaryRel, index, field, ease);
    }

    void emitSetNodeProp(rio2d::Script::Index index)
    {
      rio2d::Script::Index field = Fields::index(m_hash);

      switch (field)
      {
      case Fields::kBboxheightIndex:
      case Fields::kBboxwidthIndex:
      case Fields::kHeightIndex:
      case Fields::kOpacityIndex:
      case Fields::kRotationIndex:
      case Fields::kScaleIndex:
      case Fields::kWidthIndex:
      case Fields::kXIndex:
      case Fields::kYIndex:
        match(Tokens::kIdentifier);
        match('=');

        if (parseExpression() != Tokens::kNumber)
        {
          raise(Errors::kTypeMismatch);
          return;
        }

        emit(Insns::kSetProp, index, field);
        break;

      case Fields::kVisibleIndex:
        match(Tokens::kIdentifier);
        match('=');

        if (parseExpression() != Tokens::kTrue)
        {
          raise(Errors::kTypeMismatch);
          return;
        }

        emit(Insns::kSetProp, index, field);
        break;

      case Fields::kMovebyIndex:
      case Fields::kRotatebyIndex:
      case Fields::kScalebyIndex:
        emitNodeVary(false, index);
        break;

      case Fields::kFadeinIndex:
      case Fields::kFadeoutIndex:
      case Fields::kMovetoIndex:
      case Fields::kRotatetoIndex:
      case Fields::kScaletoIndex:
        emitNodeVary(true, index);
        break;

      default:
        raise(Errors::kUnknownField);
        return;
      }
    }

    void emitSetVec2Prop(rio2d::Script::Index index)
    {
      rio2d::Script::Index field = Fields::index(m_hash);

      switch (field)
      {
      case Fields::kXIndex:
      case Fields::kYIndex:
        match(Tokens::kIdentifier);
        match('=');

        if (parseExpression() != Tokens::kNumber)
        {
          raise(Errors::kTypeMismatch);
          return;
        }

        emit(Insns::kSetProp, index, field);
        break;

      default:
        raise(Errors::kUnknownField);
        return;
      }
    }

    void emitSetSizeProp(rio2d::Script::Index index)
    {
      rio2d::Script::Index field = Fields::index(m_hash);

      switch (field)
      {
      case Fields::kHeightIndex:
      case Fields::kWidthIndex:
        match(Tokens::kIdentifier);
        match('=');

        if (parseExpression() != Tokens::kNumber)
        {
          raise(Errors::kTypeMismatch);
          return;
        }

        emit(Insns::kSetProp, index, field);
        break;

      default:
        raise(Errors::kUnknownField);
        return;
      }
    }

    int emitGetNodeProp(rio2d::Script::Index index)
    {
      rio2d::Script::Index field = Fields::index(m_hash);

      switch (field)
      {
      case Fields::kBboxheightIndex:
      case Fields::kBboxwidthIndex:
      case Fields::kHeightIndex:
      case Fields::kOpacityIndex:
      case Fields::kRotationIndex:
      case Fields::kScaleIndex:
      case Fields::kWidthIndex:
      case Fields::kXIndex:
      case Fields::kYIndex:
        match(Tokens::kIdentifier);
        emit(Insns::kGetProp, index, field);
        return Tokens::kNumber;

      case Fields::kVisibleIndex:
        match(Tokens::kIdentifier);
        emit(Insns::kGetProp, index, field);
        return Tokens::kTrue;

      default:
        return raise(Errors::kUnknownField);
      }
    }

    int emitGetVec2Prop(rio2d::Script::Index index)
    {
      rio2d::Script::Index field = Fields::index(m_hash);

      switch (field)
      {
      case Fields::kXIndex:
      case Fields::kYIndex:
        match(Tokens::kIdentifier);
        emit(Insns::kGetProp, index, field);
        return Tokens::kNumber;

      default:
        return raise(Errors::kUnknownField);
      }
    }

    int emitGetSizeProp(rio2d::Script::Index index)
    {
      rio2d::Script::Index field = Fields::index(m_hash);

      switch (field)
      {
      case Fields::kHeightIndex:
      case Fields::kWidthIndex:
        match(Tokens::kIdentifier);
        emit(Insns::kGetProp, index, field);
        return Tokens::kNumber;

      default:
        return raise(Errors::kUnknownField);
      }
    }

    void parse()
    {
      while (m_token == Tokens::kSub)
      {
        parseSub();
      }

      match(Tokens::kEof);
    }

    void parseSub()
    {
      match();

      m_emitter->addGlobal(m_hash);
      match(Tokens::kIdentifier);

      match('(');

      rio2d::Hash hash = m_hash;
      match(Tokens::kIdentifier);

      match(Tokens::kAs);

      rio2d::Hash type = m_token;

      if (type != Tokens::kNode)
      {
        raise(Errors::kFirstParamNotANode);
        return;
      }

      match();
      m_emitter->addLocal(hash, type);

      while (m_token == ',')
      {
        match();

        hash = m_hash;
        match(Tokens::kIdentifier);

        match(Tokens::kAs);

        type = m_token;

        switch (type)
        {
        case Tokens::kNode:
        case Tokens::kNumber:
        case Tokens::kSize:
        case Tokens::kVec2:
          match();
          m_emitter->addLocal(hash, type);
          break;

        default:
          raise(Errors::kUnknownType);
          return;
        }
      }

      match(')');

      for (;;)
      {
        switch (m_token)
        {
        case Tokens::kForever:    parseForever(); goto out; // Forever can only be the last statement in a define.
        case Tokens::kParallel:   parseParallel(); break;
        case Tokens::kRepeat:     parseRepeat(); break;
        case Tokens::kSequence:   parseSequence(); break;
        case Tokens::kIdentifier: parseStatement(); break;
        case Tokens::kLet:        parseLet(); break;
        case Tokens::kSignal:     parseSignal(); break;
        case Tokens::kPause:      parsePause(); break;
        default:                  emit(Insns::kStop); goto out; // Let match(kEnd) raise the error, if any.
        }
      }

    out:
      match(Tokens::kEnd);
    }

    void parseForever()
    {
      match();

      rio2d::Script::Address targetPC = m_emitter->getPC();

      for (;;)
      {
        switch (m_token)
        {
        case Tokens::kForever:    parseForever(); goto out; // Forever can only be the last statement in a "forever" sequence.
        case Tokens::kParallel:   parseParallel(); break;
        case Tokens::kRepeat:     parseRepeat(); break;
        case Tokens::kSequence:   parseSequence(); break;
        case Tokens::kIdentifier: parseStatement(); break;
        case Tokens::kLet:        parseLet(); break;
        case Tokens::kSignal:     parseSignal(); break;
        case Tokens::kPause:      parsePause(); break;
        default:                  goto out; // Let match(kEnd) raise the error, if any.
        }
      }

    out:
      match(Tokens::kEnd);
      emit(Insns::kJump, targetPC);
    }

    void parseParallel()
    {
      match();

      rio2d::Script::Address jump = m_emitter->getPC();
      emit(Insns::kJump, 0);

      rio2d::Script::Address entries[rio2d::Script::kMaxThreads - 1]; // One thread must be available to spawn the others.
      size_t count = 0;

      for (;;)
      {
        switch (m_token)
        {
        case Tokens::kForever:
        case Tokens::kParallel:
        case Tokens::kRepeat:
        case Tokens::kSequence:
          if (count == sizeof(entries) / sizeof(entries[0]))
          {
            raise(Errors::kOutOfMemory);
            return;
          }

          entries[count++] = m_emitter->getPC();
        }

        switch (m_token)
        {
        case Tokens::kForever:  parseForever(); break;
        case Tokens::kParallel: parseParallel(); emit(Insns::kStop); break;
        case Tokens::kRepeat:   parseRepeat(); emit(Insns::kStop); break;
        case Tokens::kSequence: parseSequence(); emit(Insns::kStop); break;
        default:                goto out; // Let match(kEnd) raise the error, if any.
        }
      }

    out:
      match(Tokens::kEnd);

      rio2d::Script::Bytecode bc;
      bc.m_address = m_emitter->getPC();
      m_emitter->patch(jump + 1, bc);

      for (size_t i = 0; i < count; i++)
      {
        emit(Insns::kSpawn, entries[i]);
      }
    }

    void parseRepeat()
    {
      match();

      rio2d::Script::Token type = parseExpression();

      if (type != Tokens::kNumber)
      {
        raise(Errors::kTypeMismatch);
        return;
      }

      match(Tokens::kTimes);

      rio2d::Script::Address targetPC = m_emitter->getPC();

      for (;;)
      {
        switch (m_token)
        {
        case Tokens::kForever:    parseForever(); goto out; // Forever can only be the last statement in a sequence.
        case Tokens::kParallel:   parseParallel(); break;
        case Tokens::kRepeat:     parseRepeat(); break;
        case Tokens::kSequence:   parseSequence(); break;
        case Tokens::kIdentifier: parseStatement(); break;
        case Tokens::kLet:        parseLet(); break;
        case Tokens::kSignal:     parseSignal(); break;
        case Tokens::kPause:      parsePause(); break;
        default:                  goto out; // Let match(kEnd) raise the error, if any.
        }
      }

    out:
      match(Tokens::kEnd);

      emit(Insns::kPush, 1.0f);
      emit(Insns::kSub);
      emit(Insns::kDup);
      emit(Insns::kJnz, targetPC);
      emit(Insns::kDrop);
    }

    void parseSequence()
    {
      match();

      for (;;)
      {
        switch (m_token)
        {
        case Tokens::kForever:    parseForever(); goto out; // Forever can only be the last statement in a sequence.
        case Tokens::kParallel:   parseParallel(); break;
        case Tokens::kRepeat:     parseRepeat(); break;
        case Tokens::kSequence:   parseSequence(); break;
        case Tokens::kIdentifier: parseStatement(); break;
        case Tokens::kLet:        parseLet(); break;
        case Tokens::kSignal:     parseSignal(); break;
        case Tokens::kPause:      parsePause(); break;
        default:                  goto out; // Let match(kEnd) raise the error, if any.
        }
      }

    out:
      match(Tokens::kEnd);
    }

    void parseStatement()
    {
      rio2d::Hash hash = m_hash;
      match(Tokens::kIdentifier);

      rio2d::Script::Token type;
      Errors::Enum error = m_emitter->getType(hash, &type);

      if (error != Errors::kOk)
      {
        raise(error);
        return;
      }

      rio2d::Script::Index index;
      error = m_emitter->getIndex(hash, &index);

      if (error != Errors::kOk)
      {
        raise(error);
        return;
      }

      match('.');

      switch (type)
      {
      case Tokens::kNode: emitSetNodeProp(index); break;
      case Tokens::kVec2: emitSetVec2Prop(index); break;
      case Tokens::kSize: emitSetSizeProp(index); break;
      }
    }

    void parseLet()
    {
      match();

      rio2d::Hash hash = m_hash;
      match(Tokens::kIdentifier);

      match('=');

      rio2d::Script::Token type = parseExpression();

      rio2d::Script::Token idType;
      Errors::Enum error = m_emitter->getType(hash, &idType);

      if (error == Errors::kUnknownIdentifier)
      {
        m_emitter->addLocal(hash, type);
      }
      else if (error == Errors::kOk)
      {
        if (idType != type)
        {
          raise(Errors::kTypeMismatch);
          return;
        }
      }
      else
      {
        raise(error);
        return;
      }

      rio2d::Script::Index index;
      error = m_emitter->getIndex(hash, &index);

      if (error != Errors::kOk)
      {
        raise(error);
        return;
      }

      emit(Insns::kSetGlobal, index);
    }

    void parseSignal()
    {
      match();
      rio2d::Hash hash = m_hash;
      match(Tokens::kStringConst);
      emit(Insns::kSignal, hash);
    }

    void parsePause()
    {
      match();

      if (parseExpression() != Tokens::kNumber)
      {
        raise(Errors::kTypeMismatch);
        return;
      }

      emit(Insns::kPause);
    }

    rio2d::Script::Token parseExpression()
    {
      return parseLogicalOr();
    }

    rio2d::Script::Token parseLogicalOr()
    {
      rio2d::Script::Token type1 = parseLogicalAnd();

      while (m_token == Tokens::kOr)
      {
        match();

        rio2d::Script::Token type2 = parseLogicalAnd();

        if (type1 == Tokens::kTrue && type2 == Tokens::kTrue)
        {
          emit(Insns::kLogicalOr);
        }
        else
        {
          return raise(Errors::kTypeMismatch);
        }
      }

      return type1;
    }

    rio2d::Script::Token parseLogicalAnd()
    {
      rio2d::Script::Token type1 = parseRelational();

      while (m_token == Tokens::kAnd)
      {
        match();

        rio2d::Script::Token type2 = parseRelational();

        if (type1 == Tokens::kTrue && type2 == Tokens::kTrue)
        {
          emit(Insns::kLogicalAnd);
        }
        else
        {
          return raise(Errors::kTypeMismatch);
        }
      }

      return type1;
    }

    rio2d::Script::Token parseRelational()
    {
      rio2d::Script::Token type1 = parseTerm();

      while (m_token == '=' || m_token == '<' || m_token == '>' || m_token == Tokens::kNotEqual || m_token == Tokens::kLessEqual || m_token == Tokens::kGreaterEqual)
      {
        rio2d::Script::Token op = m_token;
        match();

        rio2d::Script::Token type2 = parseTerm();

        if (type1 == Tokens::kNumber && type2 == Tokens::kNumber)
        {
          switch (op)
          {
          case '=':                   emit(Insns::kCmpEqual); break;
          case '<':                   emit(Insns::kCmpLess);  break;
          case '>':                   emit(Insns::kCmpGreater); break;
          case Tokens::kNotEqual:     emit(Insns::kCmpNotEqual); break;
          case Tokens::kLessEqual:    emit(Insns::kCmpLessEqual); break;
          case Tokens::kGreaterEqual: emit(Insns::kCmpGreaterEqual); break;
          }

          type1 = Tokens::kTrue;
        }
        else
        {
          return raise(Errors::kTypeMismatch);
        }
      }

      return type1;
    }

    rio2d::Script::Token parseTerm()
    {
      rio2d::Script::Token type1 = parseFactor();

      while (m_token == '+' || m_token == '-')
      {
        rio2d::Script::Token op = m_token;
        match();

        rio2d::Script::Token type2 = parseFactor();

        if (type1 == Tokens::kNumber && type2 == Tokens::kNumber)
        {
          switch (op)
          {
          case '+': emit(Insns::kAdd); break;
          case '-': emit(Insns::kSub); break;
          }
        }
        else
        {
          return raise(Errors::kTypeMismatch);
        }
      }

      return type1;
    }

    rio2d::Script::Token parseFactor()
    {
      rio2d::Script::Token type1 = parseUnary();

      while (m_token == '*' || m_token == '/' || m_token == Tokens::kMod)
      {
        rio2d::Script::Token op = m_token;
        match();

        rio2d::Script::Token type2 = parseUnary();

        if (type1 == Tokens::kNumber && type2 == Tokens::kNumber)
        {
          switch (op)
          {
          case '*':          emit(Insns::kMul); break;
          case '/':          emit(Insns::kDiv);  break;
          case Tokens::kMod: emit(Insns::kModulus); break;
          }
        }
        else
        {
          return raise(Errors::kTypeMismatch);
        }
      }

      return type1;
    }

    rio2d::Script::Token parseUnary()
    {
      rio2d::Script::Token type;

      switch (m_token)
      {
      case '-':
        match();
        type = parseTerminal();

        if (type != Tokens::kNumber)
        {
          return raise(Errors::kTypeMismatch);
        }

        emit(Insns::kNeg);
        break;

      case '+':
        match();
        type = parseTerminal();

        if (type != Tokens::kNumber)
        {
          return raise(Errors::kTypeMismatch);
        }

        break;

      case Tokens::kNot:
        match();
        type = parseTerminal();

        if (type != Tokens::kTrue)
        {
          return raise(Errors::kTypeMismatch);
        }

        emit(Insns::kLogicalNot);
        break;

      default:
        type = parseTerminal();
        break;
      }

      return type;
    }

    rio2d::Script::Token parseTerminal()
    {
      switch (m_token)
      {
      case Tokens::kNumberConst:
      {
        char* end;
        rio2d::Script::Number number = strtof(m_lexeme, &end);

        if (end - m_lexeme != m_length)
        {
          return raise(Errors::kMalformedNumber);
        }

        match();
        emit(Insns::kPush, number);
        return Tokens::kNumber;
      }

      case Tokens::kTrue:
        match();
        emit(Insns::kPush, 1.0f);
        return Tokens::kTrue;

      case Tokens::kFalse:
        match();
        emit(Insns::kPush, 0.0f);
        return Tokens::kTrue; // kTrue flags a boolean expression

      case '(':
      {
        match();
        rio2d::Script::Token type = parseExpression();
        match(')');
        return type;
      }

      case Tokens::kIdentifier:
      {
        rio2d::Script::Index index;
        Errors::Enum error = m_emitter->getIndex(m_hash, &index);

        if (error != Errors::kOk)
        {
          return raise(error);
        }

        rio2d::Script::Token type;
        m_emitter->getType(m_hash, &type);

        match();

        if (m_token == '.')
        {
          match();

          switch (type)
          {
          case Tokens::kNode: return emitGetNodeProp(index); break;
          case Tokens::kVec2: return emitGetVec2Prop(index); break;
          case Tokens::kSize: return emitGetSizeProp(index); break;
          }
        }
        else
        {
          emit(Insns::kGetGlobal, index);
          return type;
        }
      }

      case Tokens::kRand:
        match();

        if (m_token == '(')
        {
          match();

          if (parseExpression() != Tokens::kNumber)
          {
            return raise(Errors::kTypeMismatch);
          }

          match(',');

          if (parseExpression() != Tokens::kNumber)
          {
            return raise(Errors::kTypeMismatch);
          }

          match(')');
          emit(Insns::kRandRange);
        }
        else
        {
          emit(Insns::kRand);
        }

        return Tokens::kNumber;

      case Tokens::kFloor:
      {
        match();
        match('(');
        rio2d::Script::Token type = parseExpression();
        match(')');
        emit(Insns::kFloor);
        return type;
      }

      case Tokens::kCeil:
      {
        match();
        match('(');
        rio2d::Script::Token type = parseExpression();
        match(')');
        emit(Insns::kCeil);
        return type;
      }

      case Tokens::kTrunc:
      {
        match();
        match('(');
        rio2d::Script::Token type = parseExpression();
        match(')');
        emit(Insns::kTrunc);
        return type;
      }
      }

      return raise(Errors::kUnexpectedToken);
    }
  };

  class Runner : public cocos2d::ActionInterval
  {
  protected:
    struct Thread
    {
      rio2d::Script::Address m_pc;
      float m_dt;
      unsigned m_sp;
      rio2d::Script::Number m_stack[rio2d::Script::kMaxStack];
    };

    cocos2d::Ref* m_owner;
    rio2d::Script::LocalVar* m_locals;
    size_t m_numLocals;
    const rio2d::Script::Bytecode* m_bytecode;
    Thread m_threads[rio2d::Script::kMaxThreads];
    size_t m_numThreads;
    cocos2d::Ref* m_listener;
    rio2d::Script::NotifyFunc m_port;

    ~Runner()
    {
      delete[] m_locals;
      m_owner->release();
    }

  public:
    static Runner* create(cocos2d::Ref* owner, rio2d::Script::LocalVar* locals, size_t numLocals, const rio2d::Script::Bytecode* bytecode, rio2d::Script::Address pc, cocos2d::Ref* listener, rio2d::Script::NotifyFunc port, cocos2d::Node* target, va_list args)
    {
      Runner *self = new (std::nothrow) Runner();

      if (self && self->init(owner, locals, numLocals, bytecode, pc, listener, port, target, args))
      {
        self->autorelease();
        owner->retain();
        return self;
      }

      CC_SAFE_DELETE(self);
      return nullptr;
    }

    void step(float dt)
    {
      for (unsigned i = 0; i < m_numThreads; i++)
      {
        Thread* thread = m_threads + i;
        thread->m_dt += dt;
        consume(thread);
      }

      Thread* t1 = m_threads;
      const Thread* t2 = m_threads;
      const Thread* end = t2 + m_numThreads;

      while (t2 < end)
      {
        if (t1 != t2 && m_bytecode[t2->m_pc].m_insn != Insns::kStop)
        {
          *t1 = *t2;
        }

        if (m_bytecode[t1->m_pc].m_insn != Insns::kStop)
        {
          t1++;
        }

        t2++;
      }

      m_numThreads = t1 - m_threads;
    }

    void update(float time)
    {
      (void)time; // What should we do here???
    }

    bool isDone() const
    {
      return m_numThreads == 0;
    }

  protected:
    bool init(cocos2d::Ref* owner, rio2d::Script::LocalVar* locals, size_t numLocals, const rio2d::Script::Bytecode* bytecode, size_t pc, cocos2d::Ref* listener, rio2d::Script::NotifyFunc port, cocos2d::Node* target, va_list args)
    {
      m_locals = new rio2d::Script::LocalVar[numLocals];

      if (m_locals == nullptr)
      {
        return false;
      }

      memcpy(m_locals, locals, sizeof(rio2d::Script::LocalVar) * numLocals);
      m_numLocals = numLocals;

      m_bytecode = bytecode;

      m_threads[0].m_pc = pc;
      m_threads[0].m_dt = 0.0f;
      m_threads[0].m_sp = 0;

      m_listener = listener;
      m_port = port;

      m_numThreads = 1;

      rio2d::Script::LocalVar* local = m_locals;
      const rio2d::Script::LocalVar* end = local + m_numLocals;

      local->m_pointer = target;
      local++;

      while (local < end)
      {
        switch (local->m_type)
        {
        case Tokens::kNode:   local->m_pointer = va_arg(args, cocos2d::Node*); break;
        case Tokens::kNumber: local->m_number = (rio2d::Script::Number)va_arg(args, double); break;
        case Tokens::kSize:   local->m_pointer = va_arg(args, cocos2d::Size*); break;
        case Tokens::kVec2:   local->m_pointer = va_arg(args, cocos2d::Vec2*); break;
        }

        local++;
      }

      m_owner = owner;
      return true;
    }

    bool add(Thread* thread)
    {
      thread->m_stack[thread->m_sp - 2] += thread->m_stack[thread->m_sp - 1];
      thread->m_sp--;
      return true; // Continue running
    }

    void callNodeMethod(Thread* thread, cocos2d::Node* target, rio2d::Hash hash)
    {
      (void)thread;
      (void)target;
      (void)hash;
    }

    bool callMethod(Thread* thread)
    {
      rio2d::Script::LocalVar* local = m_locals + m_bytecode[thread->m_pc++].m_index;
      rio2d::Script::Index index = m_bytecode[thread->m_pc++].m_index;

      switch (local->m_type)
      {
      case Tokens::kNode:
        callNodeMethod(thread, (cocos2d::Node*)local->m_pointer, index);
        break;

      default:
        CCASSERT(0, "Unknown object type");
      }

      return true;
    }

    bool ceil(Thread* thread)
    {
      thread->m_stack[thread->m_sp - 1] = ::ceil(thread->m_stack[thread->m_sp - 1]);
      return true;
    }

    bool cmpEqual(Thread* thread)
    {
      thread->m_stack[thread->m_sp - 2] = thread->m_stack[thread->m_sp - 2] == thread->m_stack[thread->m_sp - 1];
      thread->m_sp--;
      return true;
    }

    bool cmpGreater(Thread* thread)
    {
      thread->m_stack[thread->m_sp - 2] = thread->m_stack[thread->m_sp - 2] > thread->m_stack[thread->m_sp - 1];
      thread->m_sp--;
      return true;
    }

    bool cmpGreaterEqual(Thread* thread)
    {
      thread->m_stack[thread->m_sp - 2] = thread->m_stack[thread->m_sp - 2] >= thread->m_stack[thread->m_sp - 1];
      thread->m_sp--;
      return true;
    }

    bool cmpLess(Thread* thread)
    {
      thread->m_stack[thread->m_sp - 2] = thread->m_stack[thread->m_sp - 2] < thread->m_stack[thread->m_sp - 1];
      thread->m_sp--;
      return true;
    }

    bool cmpLessEqual(Thread* thread)
    {
      thread->m_stack[thread->m_sp - 2] = thread->m_stack[thread->m_sp - 2] <= thread->m_stack[thread->m_sp - 1];
      thread->m_sp--;
      return true;
    }

    bool cmpNotEqual(Thread* thread)
    {
      thread->m_stack[thread->m_sp - 2] = thread->m_stack[thread->m_sp - 2] != thread->m_stack[thread->m_sp - 1];
      thread->m_sp--;
      return true;
    }

    bool div(Thread* thread)
    {
      thread->m_stack[thread->m_sp - 2] /= thread->m_stack[thread->m_sp - 1];
      thread->m_sp--;
      return true;
    }

    bool drop(Thread* thread)
    {
      thread->m_sp--;
      return true;
    }

    bool dup(Thread* thread)
    {
      thread->m_stack[thread->m_sp] = thread->m_stack[thread->m_sp - 1];
      thread->m_sp++;
      return true;
    }

    bool floor(Thread* thread)
    {
      thread->m_stack[thread->m_sp - 1] = ::floor(thread->m_stack[thread->m_sp - 1]);
      return true;
    }

    bool getGlobal(Thread* thread)
    {
      rio2d::Script::LocalVar* local = m_locals + m_bytecode[thread->m_pc++].m_index;
      thread->m_stack[thread->m_sp++] = local->m_number;
      return true;
    }

    void getNodeProp(Thread* thread, cocos2d::Node* obj, rio2d::Script::Index field)
    {
      float value;

      switch (field)
      {
      case Fields::kBboxheightIndex: value = obj->getBoundingBox().size.height; break;
      case Fields::kBboxwidthIndex:  value = obj->getBoundingBox().size.width; break;
      case Fields::kHeightIndex:     value = obj->getContentSize().height; break;
      case Fields::kOpacityIndex:    value = obj->getOpacity(); break;
      case Fields::kRotationIndex:   value = obj->getRotation(); break;
      case Fields::kScaleIndex:      value = obj->getScale(); break;
      case Fields::kWidthIndex:      value = obj->getContentSize().width; break;
      case Fields::kXIndex:          value = obj->getPositionX(); break;
      case Fields::kYIndex:          value = obj->getPositionY(); break;
      case Fields::kVisibleIndex:    value = obj->isVisible() ? 1.0f : 0.0f; break;
      default:                       CCASSERT(0, "Unknown property for Node instance");
      }

      thread->m_stack[thread->m_sp++] = value;
    }

    void getVec2Prop(Thread* thread, cocos2d::Vec2* obj, rio2d::Script::Index field)
    {
      float value;

      switch (field)
      {
      case Fields::kXIndex: value = obj->x; break;
      case Fields::kYIndex: value = obj->y; break;
      default:              CCASSERT(0, "Unknown property for Vec2 instance");
      }

      thread->m_stack[thread->m_sp++] = value;
    }

    void getSizeProp(Thread* thread, cocos2d::Size* obj, rio2d::Script::Index field)
    {
      float value;

      switch (field)
      {
      case Fields::kHeightIndex: value = obj->height; break;
      case Fields::kWidthIndex:  value = obj->width; break;
      default:                   CCASSERT(0, "Unknown property for Size instance");
      }

      thread->m_stack[thread->m_sp++] = value;
    }

    bool getProp(Thread* thread)
    {
      rio2d::Script::LocalVar* local = m_locals + m_bytecode[thread->m_pc++].m_index;
      rio2d::Script::Index field = m_bytecode[thread->m_pc++].m_index;

      switch (local->m_type)
      {
      case Tokens::kNode:
        getNodeProp(thread, (cocos2d::Node*)local->m_pointer, field);
        break;

      case Tokens::kSize:
        getSizeProp(thread, (cocos2d::Size*)local->m_pointer, field);
        break;

      case Tokens::kVec2:
        getVec2Prop(thread, (cocos2d::Vec2*)local->m_pointer, field);
        break;

      default:
        CCASSERT(0, "Unknown object type");
      }

      return true;
    }

    bool jnz(Thread* thread)
    {
      if (thread->m_stack[--thread->m_sp] != 0.0f)
      {
        thread->m_pc = m_bytecode[thread->m_pc].m_address;
      }
      else
      {
        thread->m_pc++;
      }

      return true;
    }

    bool jump(Thread* thread)
    {
      thread->m_pc = m_bytecode[thread->m_pc].m_address;
      return true;
    }

    bool logicalAnd(Thread* thread)
    {
      thread->m_stack[thread->m_sp - 2] = thread->m_stack[thread->m_sp - 2] != 0.0f && thread->m_stack[thread->m_sp - 1] != 0.0f;
      thread->m_sp--;
      return true;
    }

    bool logicalNot(Thread* thread)
    {
      thread->m_stack[thread->m_sp - 1] = thread->m_stack[thread->m_sp - 1] != 0.0f ? 0.0f : 1.0f;
      return true;
    }

    bool logicalOr(Thread* thread)
    {
      thread->m_stack[thread->m_sp - 2] = thread->m_stack[thread->m_sp - 2] != 0.0f || thread->m_stack[thread->m_sp - 1] != 0.0f;
      thread->m_sp--;
      return true;
    }

    bool modulus(Thread* thread)
    {
      thread->m_stack[thread->m_sp - 2] = fmod(thread->m_stack[thread->m_sp - 2], thread->m_stack[thread->m_sp - 1] != 0.0f);
      thread->m_sp--;
      return true;
    }

    bool mul(Thread* thread)
    {
      thread->m_stack[thread->m_sp - 2] *= thread->m_stack[thread->m_sp - 1];
      thread->m_sp--;
      return true;
    }

    bool neg(Thread* thread)
    {
      thread->m_stack[thread->m_sp - 1] = -thread->m_stack[thread->m_sp - 1];
      return true;
    }

    bool pause(Thread* thread)
    {
      float time = thread->m_stack[thread->m_sp - 1] -= thread->m_dt;

      if (time > 0.0f)
      {
        return false; // Put this thread to sleep.
      }

      thread->m_dt = -time;
      thread->m_sp--;
      return true;
    }

    bool push(Thread* thread)
    {
      thread->m_stack[thread->m_sp++] = m_bytecode[thread->m_pc++].m_number;
      return true;
    }

    inline float rnd()
    {
      return (float)::rand() / (float)(RAND_MAX + 1);
    }

    bool rand(Thread* thread)
    {
      thread->m_stack[thread->m_sp++] = rnd();
      return true;
    }

    bool randRange(Thread* thread)
    {
      float a = ::floor(thread->m_stack[thread->m_sp - 2]);
      float b = ::floor(thread->m_stack[thread->m_sp - 1]) + 1.0f;

      thread->m_stack[thread->m_sp - 2] = ::floor(a + rnd() * (b - a));
      thread->m_sp--;
      return true;
    }

    bool setGlobal(Thread* thread)
    {
      rio2d::Script::LocalVar* local = m_locals + m_bytecode[thread->m_pc++].m_index;
      local->m_number = thread->m_stack[--thread->m_sp];
      return true;
    }

    void setNodeProp(Thread* thread, cocos2d::Node* obj, rio2d::Script::Index field)
    {
      rio2d::Script::Number value = thread->m_stack[--thread->m_sp];

      switch (field)
      {
      case Fields::kOpacityIndex:  obj->setOpacity(value); break;
      case Fields::kRotationIndex: obj->setRotation(value); break;
      case Fields::kScaleIndex:    obj->setScale(value); break;
      case Fields::kXIndex:        obj->setPositionX(value); break;
      case Fields::kYIndex:        obj->setPositionY(value); break;
      case Fields::kVisibleIndex:  obj->setVisible(value != 0.0f); break;
      default:                     CCASSERT(0, "Unknown Node property");
      }
    }

    bool setProp(Thread* thread)
    {
      rio2d::Script::LocalVar* local = m_locals + m_bytecode[thread->m_pc++].m_index;
      rio2d::Script::Index field = m_bytecode[thread->m_pc++].m_index;

      switch (local->m_type)
      {
      case Tokens::kNode:
        setNodeProp(thread, (cocos2d::Node*)local->m_pointer, field);
        break;

      default:
        CCASSERT(0, "Unknown object type");
      }

      return true;
    }

    bool signal(Thread* thread)
    {
      rio2d::Hash hash = m_bytecode[thread->m_pc++].m_hash;

      if (m_listener != nullptr)
      {
        cocos2d::Node* target = (cocos2d::Node*)m_locals->m_pointer;
        (m_listener->*m_port)(target, hash);
      }

      return true;
    }

    bool spawn(Thread* thread)
    {
      if (m_numThreads < rio2d::Script::kMaxThreads)
      {
        Thread* nt = m_threads + m_numThreads++;
        nt->m_pc = m_bytecode[thread->m_pc++].m_address;
        nt->m_sp = 0;
        nt->m_dt = thread->m_dt;
      }

      return true;
    }

    bool stop(Thread* thread)
    {
      return false;
    }

    bool sub(Thread* thread)
    {
      thread->m_stack[thread->m_sp - 2] -= thread->m_stack[thread->m_sp - 1];
      thread->m_sp--;
      return true;
    }

    bool trunc(Thread* thread)
    {
      thread->m_stack[thread->m_sp - 1] = ::trunc(thread->m_stack[thread->m_sp - 1]);
      return true;
    }

    bool varyAbs(Thread* thread)
    {
      struct Args1
      {
        rio2d::Script::Number m_sourceA;
        rio2d::Script::Number m_destA;
        rio2d::Script::Number m_sourceT;
        rio2d::Script::Number m_destT;
      };

      struct Args2
      {
        rio2d::Script::Number m_sourceA;
        rio2d::Script::Number m_sourceB;
        rio2d::Script::Number m_destA;
        rio2d::Script::Number m_destB;
        rio2d::Script::Number m_sourceT;
        rio2d::Script::Number m_destT;
      };

      rio2d::Script::Index index = m_bytecode[thread->m_pc].m_index;
      rio2d::Script::Index field = m_bytecode[thread->m_pc + 1].m_index;
      rio2d::Script::Index ease = m_bytecode[thread->m_pc + 2].m_index;

      rio2d::Script::LocalVar* local = m_locals + index;
      cocos2d::Node* node = (cocos2d::Node*)local->m_pointer;

      switch (field)
      {
      case Fields::kOpacityIndex:
      {
        Args1* args = (Args1*)((char*)(thread->m_stack + thread->m_sp) - sizeof(Args1));
        args->m_sourceT += thread->m_dt;

        if (args->m_sourceT < args->m_destT)
        {
          rio2d::Script::Number time = Easing::evaluate(ease, args->m_sourceT / args->m_destT);
          node->setOpacity(args->m_sourceA + time * (args->m_destA - args->m_sourceA));
          return false;
        }
        else
        {
          node->setOpacity(args->m_destA);
          thread->m_dt = args->m_sourceT - args->m_destT;
          thread->m_sp -= 4;
          thread->m_pc += 3;
          return true;
        }

        break;
      }

      case Fields::kPositionIndex:
      {
        Args2* args = (Args2*)((char*)(thread->m_stack + thread->m_sp) - sizeof(Args2));
        args->m_sourceT += thread->m_dt;

        if (args->m_sourceT < args->m_destT)
        {
          rio2d::Script::Number time = Easing::evaluate(ease, args->m_sourceT / args->m_destT);
          node->setPositionX(args->m_sourceA + time * (args->m_destA - args->m_sourceA));
          node->setPositionY(args->m_sourceB + time * (args->m_destB - args->m_sourceB));
          return false;
        }
        else
        {
          node->setPositionX(args->m_destA);
          node->setPositionY(args->m_destB);
          thread->m_dt = args->m_sourceT - args->m_destT;
          thread->m_sp -= 6;
          thread->m_pc += 3;
          return true;
        }

        break;
      }

      case Fields::kRotationIndex:
      {
        Args1* args = (Args1*)((char*)(thread->m_stack + thread->m_sp) - sizeof(Args1));
        args->m_sourceT += thread->m_dt;

        if (args->m_sourceT < args->m_destT)
        {
          rio2d::Script::Number time = Easing::evaluate(ease, args->m_sourceT / args->m_destT);
          node->setRotation(args->m_sourceA + time * (args->m_destA - args->m_sourceA));
          return false;
        }
        else
        {
          node->setRotation(args->m_destA);
          thread->m_dt = args->m_sourceT - args->m_destT;
          thread->m_sp -= 4;
          thread->m_pc += 3;
          return true;
        }

        break;
      }

      case Fields::kScaleIndex:
      {
        Args1* args = (Args1*)((char*)(thread->m_stack + thread->m_sp) - sizeof(Args1));
        args->m_sourceT += thread->m_dt;

        if (args->m_sourceT < args->m_destT)
        {
          rio2d::Script::Number time = Easing::evaluate(ease, args->m_sourceT / args->m_destT);
          node->setScale(args->m_sourceA + time * (args->m_destA - args->m_sourceA));
          return false;
        }
        else
        {
          node->setScale(args->m_destA);
          thread->m_dt = args->m_sourceT - args->m_destT;
          thread->m_sp -= 4;
          thread->m_pc += 3;
          return true;
        }

        break;
      }
      }

      return true;
    }

    bool varyRel(Thread* thread)
    {
      struct Args1
      {
        rio2d::Script::Number m_sourceA;
        rio2d::Script::Number m_ratioA;
        rio2d::Script::Number m_sourceT;
        rio2d::Script::Number m_destT;
      };

      struct Args2
      {
        rio2d::Script::Number m_sourceA;
        rio2d::Script::Number m_sourceB;
        rio2d::Script::Number m_ratioA;
        rio2d::Script::Number m_ratioB;
        rio2d::Script::Number m_sourceT;
        rio2d::Script::Number m_destT;
      };

      rio2d::Script::Index index = m_bytecode[thread->m_pc].m_index;
      rio2d::Script::Index field = m_bytecode[thread->m_pc + 1].m_index;
      rio2d::Script::Index ease = m_bytecode[thread->m_pc + 2].m_index;

      rio2d::Script::LocalVar* local = m_locals + index;
      cocos2d::Node* node = (cocos2d::Node*)local->m_pointer;

      switch (field)
      {
      case Fields::kPositionIndex:
      {
        Args2* args = (Args2*)((char*)(thread->m_stack + thread->m_sp) - sizeof(Args2));
        args->m_sourceT += thread->m_dt;

        if (args->m_sourceT < args->m_destT)
        {
          rio2d::Script::Number time = Easing::evaluate(ease, args->m_sourceT / args->m_destT);
          node->setPositionX(args->m_sourceA + time * args->m_ratioA);
          node->setPositionY(args->m_sourceB + time * args->m_ratioB);
          return false;
        }
        else
        {
          node->setPositionX(args->m_sourceA + args->m_ratioA);
          node->setPositionY(args->m_sourceB + args->m_ratioB);
          thread->m_dt = args->m_sourceT - args->m_destT;
          thread->m_sp -= 6;
          thread->m_pc += 3;
          return true;
        }

        break;
      }

      case Fields::kRotationIndex:
      {
        Args1* args = (Args1*)((char*)(thread->m_stack + thread->m_sp) - sizeof(Args1));
        args->m_sourceT += thread->m_dt;

        if (args->m_sourceT < args->m_destT)
        {
          rio2d::Script::Number time = Easing::evaluate(ease, args->m_sourceT / args->m_destT);
          node->setRotation(args->m_sourceA + time * args->m_ratioA);
          return false;
        }
        else
        {
          node->setRotation(args->m_sourceA + args->m_ratioA);
          thread->m_dt = args->m_sourceT - args->m_destT;
          thread->m_sp -= 4;
          thread->m_pc += 3;
          return true;
        }

        break;
      }

      case Fields::kScaleIndex:
      {
        Args1* args = (Args1*)((char*)(thread->m_stack + thread->m_sp) - sizeof(Args1));
        args->m_sourceT += thread->m_dt;

        if (args->m_sourceT < args->m_destT)
        {
          rio2d::Script::Number time = Easing::evaluate(ease, args->m_sourceT / args->m_destT);
          node->setScale(args->m_sourceA + time * args->m_ratioA);
          return false;
        }
        else
        {
          node->setScale(args->m_sourceA + args->m_ratioA);
          thread->m_dt = args->m_sourceT - args->m_destT;
          thread->m_sp -= 4;
          thread->m_pc += 3;
          return true;
        }

        break;
      }
      }

      return true;
    }

    void consume(Thread* thread)
    {
      // Execute a thread until an insn that consumes time.
      // If the thread executes the same address twice while in here, stop it (infinite loop).

      rio2d::Script::Address saved_pc = thread->m_pc;

      do
      {
        bool cont;

        switch (m_bytecode[thread->m_pc++].m_insn)
        {
        case Insns::kAdd:             cont = add(thread); break;
        case Insns::kCallMethod:      cont = callMethod(thread); break;
        case Insns::kCeil:            cont = ceil(thread); break;
        case Insns::kCmpEqual:        cont = cmpEqual(thread); break;
        case Insns::kCmpGreater:      cont = cmpGreater(thread); break;
        case Insns::kCmpGreaterEqual: cont = cmpGreaterEqual(thread); break;
        case Insns::kCmpLess:         cont = cmpLess(thread); break;
        case Insns::kCmpLessEqual:    cont = cmpLessEqual(thread); break;
        case Insns::kCmpNotEqual:     cont = cmpNotEqual(thread); break;
        case Insns::kDiv:             cont = div(thread); break;
        case Insns::kDrop:            cont = drop(thread); break;
        case Insns::kDup:             cont = dup(thread); break;
        case Insns::kFloor:           cont = floor(thread); break;
        case Insns::kGetGlobal:       cont = getGlobal(thread); break;
        case Insns::kGetProp:         cont = getProp(thread); break;
        case Insns::kJnz:             cont = jnz(thread); break;
        case Insns::kJump:            cont = jump(thread); break;
        case Insns::kLogicalAnd:      cont = logicalAnd(thread); break;
        case Insns::kLogicalNot:      cont = logicalNot(thread); break;
        case Insns::kLogicalOr:       cont = logicalOr(thread); break;
        case Insns::kModulus:         cont = modulus(thread); break;
        case Insns::kMul:             cont = mul(thread); break;
        case Insns::kNeg:             cont = neg(thread); break;
        case Insns::kPause:           cont = pause(thread); break;
        case Insns::kPush:            cont = push(thread); break;
        case Insns::kRand:            cont = rand(thread); break;
        case Insns::kRandRange:       cont = randRange(thread); break;
        case Insns::kSetGlobal:       cont = setGlobal(thread); break;
        case Insns::kSetProp:         cont = setProp(thread); break;
        case Insns::kSignal:          cont = signal(thread); break;
        case Insns::kSpawn:           cont = spawn(thread); break;
        case Insns::kStop:            cont = stop(thread); break;
        case Insns::kSub:             cont = sub(thread); break;
        case Insns::kTrunc:           cont = trunc(thread); break;
        case Insns::kVaryAbs:         cont = varyAbs(thread); break;
        case Insns::kVaryRel:         cont = varyRel(thread); break;
        }

        if (!cont)
        {
          thread->m_pc--;
          thread->m_dt = 0.0f;
          return;
        }

        if (thread->m_pc == saved_pc)
        {
          // Avoid infinite loops.
          thread->m_dt = 0.0f;
          return;
        }
      } while (thread->m_dt > 0.0f);
    }
  };
}

rio2d::Script* rio2d::Script::initWithSource(const char* source, char* error, size_t size)
{
  Script *self = new (std::nothrow) Script();

  if (self && self->init(source, error, size))
  {
    self->autorelease();
    return self;
  }

  CC_SAFE_DELETE(self);
  return nullptr;
}

bool rio2d::Script::runAction(Hash hash, cocos2d::Node* target, ...)
{
  va_list args;
  va_start(args, target);

  bool res = runActionV(nullptr, nullptr, hash, target, args);

  va_end(args);
  return res;
}

bool rio2d::Script::runAction(const char* name, cocos2d::Node* target, ...)
{
  va_list args;
  va_start(args, target);

  bool res = runActionV(nullptr, nullptr, rio2d::Hash(name), target, args);

  va_end(args);
  return res;
}

bool rio2d::Script::runActionWithListener(cocos2d::Ref* listener, NotifyFunc port, Hash hash, cocos2d::Node* target, ...)
{
  va_list args;
  va_start(args, target);

  bool res = runActionV(listener, port, hash, target, args);

  va_end(args);
  return res;
}

bool rio2d::Script::runActionWithListener(cocos2d::Ref* listener, NotifyFunc port, const char* name, cocos2d::Node* target, ...)
{
  va_list args;
  va_start(args, target);

  bool res = runActionV(listener, port, hash(name), target, args);

  va_end(args);
  return res;
}

bool rio2d::Script::runActionV(cocos2d::Ref* listener, NotifyFunc port, Hash hash, cocos2d::Node* target, va_list args)
{
  Subroutine* global = m_globals;
  const Subroutine* end = global + m_numGlobals;

  while (global < end)
  {
    if (global->m_hash == hash)
    {
      Runner* action = Runner::create(this, global->m_locals, global->m_numLocals, m_bytecode, global->m_pc, listener, port, target, args);
      target->runAction(action);
      return true;
    }

    global++;
  }

  return false;
}

bool rio2d::Script::init(const char* source, char* error, size_t size)
{
  Parser parser;

  int res = parser.initWithSourceAndPointers(source, &m_bytecode, &m_bcSize, &m_globals, &m_numGlobals);

  if (res == Errors::kOk)
  {
    return true;
  }

  if (error != nullptr)
  {
    const char* msg;

    switch (res)
    {
    case Errors::kMalformedNumber:         msg = "Malformed number"; break;
    case Errors::kUnterminatedString:      msg = "Unterminated string"; break;
    case Errors::kUnexpectedEOF:           msg = "Unexpected end of file"; break;
    case Errors::kInvalidCharacterInInput: msg = "Invalid character in input"; break;
    case Errors::kUnexpectedToken:         msg = "Unexpected token"; break;
    case Errors::kOutOfMemory:             msg = "Out of memory"; break;
    case Errors::kUnknownType:             msg = "Unknown type"; break;
    case Errors::kDuplicateIdentifier:     msg = "Duplicated identifier"; break;
    case Errors::kUnknownIdentifier:       msg = "Unknown identifier"; break;
    case Errors::kUnknownField:            msg = "Unknown field"; break;
    case Errors::kTypeMismatch:            msg = "Type mismatch"; break;
    case Errors::kFirstParamNotANode:      msg = "First parameter must be of type \"node\""; break;
    case Errors::kUnknownEaseFunc:         msg = "Unknown easing function"; break;
    default:                               msg = "Unknown error"; break;
    }

    size_t length, i;
    const char* lexeme = parser.getLexeme(&length);
    char print[64];

    for (i = 0; i < std::min<>(length, sizeof(print) - 1); i++)
    {
      print[i] = *lexeme++;
    }

    print[i] = 0;

    snprintf(error, size, "%s in line %u (%s)", msg, parser.getLine(), print);
    error[size - 1] = 0;
  }

  return false;
}
