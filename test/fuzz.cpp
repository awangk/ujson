/*
 * Copyright (c) 2014 Anders Wang Kristensen
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

#include <ujson/ujson.hpp>

#include <cstdio>
#include <cstdlib>

int main(int argc, const char *argv[]) {
    try {

        std::FILE *file = nullptr;
        if (argc == 1) {
            file = stdin;     
        } else if (argc == 2) {
            file = std::fopen(argv[1], "rb");
            if (!file) {
                std::printf("unable to open '%s'\n", argv[1]);
                return EXIT_FAILURE;
            }
        }
        else {
            std::printf("ujson_fuzz {<} filename.json\n");
            return EXIT_FAILURE;
        }

        std::string json;
        for (;;) {
            char buf[64];
            auto read = std::fread(buf, 1, sizeof(buf), file);
            json.append(buf, buf + read);
            if (std::feof(file))
                break;
        }

        if (file != stdin)
            std::fclose(file);

        ujson::parse(json);
        return EXIT_SUCCESS;

    } catch (std::exception const &e) {
        std::printf("ERROR: %s\n", e.what());
        return EXIT_FAILURE;
    } catch (...) {
        std::printf("ERROR: Unknown exception.\n");
        return EXIT_FAILURE;
    }
}
