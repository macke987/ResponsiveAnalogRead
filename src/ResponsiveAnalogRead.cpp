/*
 * ResponsiveAnalogRead.cpp
 * Arduino library for eliminating noise in analogRead inputs without decreasing responsiveness
 *
 * Copyright (c) 2016 Damien Clarke
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.  
 */

#include <Arduino.h>
#include "ResponsiveAnalogRead.h"

ResponsiveAnalogRead::ResponsiveAnalogRead(int pin, bool sleepEnable, float snapMultiplier)
{
  pinMode(pin, INPUT ); // ensure button pin is an input
  digitalWrite(pin, LOW ); // ensure pullup is off on button pin
  
  this->pin = pin;
  this->sleepEnable = sleepEnable;
  setSnapMultiplier(snapMultiplier);
}

void ResponsiveAnalogRead::update()
{
  rawValue = analogRead(pin);
  prevResponsiveValue = responsiveValue;
  responsiveValue = getResponsiveValue(rawValue);
  responsiveValueHasChanged = responsiveValue != prevResponsiveValue;
}

int ResponsiveAnalogRead::getResponsiveValue(int newValue)
{
  // get current milliseconds
  unsigned long ms = millis();

  // get difference between new input value and current smooth value
  unsigned int diff = abs(newValue - smoothValue);
  
  // if sleep has been enabled, keep track of when we're asleep or not by marking the time of last activity
  // and testing to see how much time has passed since then
  if(sleepEnable) {
    unsigned int activityThreshold = sleeping ? sleepActivityThreshold : awakeActivityThreshold;

    // if diff is greater than the threshold,
    // indicate that activity has recently occured
    if(diff > activityThreshold) {
      lastActivityMS = ms;
    }

    // recalculate sleeping status
    // (asleep if last activity was over sleepDelayMS ago)
    sleeping = lastActivityMS + sleepDelayMS < ms;
  }
  
  // if we're allowed to sleep, and we're sleeping
  // then don't update responsiveValue this loop
  // just output the existing responsiveValue
  if(sleepEnable && sleeping) {
    return (int)smoothValue;
  }

  // use a 'snap curve' function, where we pass in the diff (x) and get back a number from 0-1.
  // We want small values of x to result in an output close to zero, so when the smooth value is close to the input value
  // it'll smooth out noise aggressively by responding slowly to sudden changes.
  // We want a small increase in x to result in a much higher output value, so medium and large movements are snappy and responsive,
  // and aren't made sluggish by unnecessarily filtering out noise. A hyperbola (f(x) = 1/x) curve is used.
  // First x has an offset of 1 applied, so x = 0 now results in a value of 1 from the hyperbola function.
  // High values of x tend toward 0, but we want an output that begins at 0 and tends toward 1, so 1-y flips this up the right way.
  // Finally the result is multiplied by 2 and capped at a maximum of one, which means that at a certain point all larger movements are maximally snappy

  // then multiply the input by SNAP_MULTIPLER so input values fit the snap curve better. 
  float snap = snapCurve(diff * snapMultiplier);
  
  // when sleep is enabled, the emphasis is stopping on a responsiveValue quickly, and it's less about easing into position.
  // If sleep is enabled, add a small amount to snap so it'll tend to snap into a more accurate position before sleeping starts.
  if(sleepEnable) {
    snap *= 0.8 + 0.2;
  }
  
  // calculate the exponential moving average based on the snap
  smoothValue += (newValue - smoothValue) * snap;
  
  // expected output is an integer
  return (int)smoothValue;
}

float ResponsiveAnalogRead::snapCurve(float x)
{
  float y = 1.0 / (x + 1.0);
  y = (1.0 - y) * 2.0;
  if(y > 1.0) {
    return 1.0;
  }
  return y;
}

void ResponsiveAnalogRead::setSnapMultiplier(float newMultiplier)
{
  if(newMultiplier > 1.0) {
    newMultiplier = 1.0;
  }
  if(newMultiplier < 0.0) {
    newMultiplier = 0.0;
  }
  snapMultiplier = newMultiplier;
}