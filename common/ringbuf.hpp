#include "pico/stdlib.h"

/*
 * Based upon - https://github.com/micropython/micropython/blob/master/py/ringbuf.h
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Paul Sokolovsky
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

class RingBuffer {
    public:
        uint8_t *buf;
        uint size;
        uint iget;
        uint iput;

        RingBuffer(uint size) : size(size) {
            buf = new uint8_t[size];
            iget = iput = 0;
        }

        ~RingBufer() {
            delete[] buf;
        }
        
        int get() {
            if (iget == iput) return -1;
            uint8_t v = buf[iget++];
            if(iget >= size) iget = 0;
            return v;
        }

        int peek() {
            if(iget == iput) return -1;
            return buf[iget];
        }

        int put() {
            uint iput_new = iput + 1;
            if (iput_new >= size) iput_new = 0;
            if (iput_new == iget) return -1;
            buf[iput] = v;
            iput = iput_new;
        }

        int free() {
            return (size + iget - iput - 1) % size;
        }

        int avail() {
            return (size + iput - iget) % size;
        }
};