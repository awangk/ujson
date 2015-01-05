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

static ujson::value load(const char *filename) {
    std::printf("Opening '%s'..\n", filename);
    std::FILE *fp = std::fopen(filename, "rb");
    if (!fp)
        throw std::runtime_error("Error opening file.");
    std::fseek(fp, 0, SEEK_END);
    const long size = std::ftell(fp);
    std::unique_ptr<char[]> data(new char[size]);
    std::fseek(fp, 0, SEEK_SET);
    if (std::fread(data.get(), 1, size, fp) != size) {
        std::fclose(fp);
        throw std::runtime_error("Error reading file.");
    }
    std::printf("Read %ld bytes..\n", size);
    std::fclose(fp);
    return ujson::parse(data.get(), size);
}

int main(int argc, const char *argv[]) {

    if (argc != 3) {
        std::printf("compare json files:\n");
        std::printf("usage: %s <json-file-1> <json-file-2>\n", argv[0]);
        return EXIT_FAILURE;
    }

    try {

        // load files
        auto json1 = load(argv[1]);
        auto json2 = load(argv[2]);

        if (json1 == json2)
            std::printf("'%s' and '%s' are identical.\n", argv[1], argv[2]);
        else
            std::printf("'%s' and '%s' are NOT identical.\n", argv[1], argv[2]);
        return EXIT_SUCCESS;

    } catch (const std::exception &e) {
        std::printf("%s\n", e.what());
        return EXIT_FAILURE;
    }
}
