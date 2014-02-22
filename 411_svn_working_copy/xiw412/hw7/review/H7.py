#!/usr/bin/python

# Sean Hill
# Fall 2012
# CSE 411
# Homework 7
#
# Description: Translates the image segmentation code from the following website from C++ to Python:
# http://www.cs.brown.edu/~pff/segment/
#
# Instructions: The command-line usage is as follows, where "sigma" and "k" and "min" are
# algorithm parameters, "input" is the .ppm file to be segmented, and "output" is a .ppm
# file for storing the output:
#   H7.py sigma k min input output
# The output file doesn't need to exist, and the input file must be in a special ppm format
# that starts with P6. See http://orion.math.iastate.edu/burkardt/data/ppm/ppm.html for more
# information and http://www.cs.cornell.edu/courses/cs664/2003fa/images/ for more sample
# input files. Here's an example to run the program using the files I provided:
#   H7.py .5 500 50 sign_1.ppm output.ppm

import sys, math, random, struct

MAXVAL = 255 # maximum allowed third value in P6 ppm file
WIDTH = 4.0 # used in filter calculations

# -------------------------------------------------------
# pnmfile.h
# -------------------------------------------------------

def error(message):
    """ Report pnm_error and exit. """
    sys.stderr.write("ERROR: " + message + " Exiting.\n")
    sys.exit()

def pnm_read(f):
    """ Read a line of the PNM field, skipping comments. """
    c = f.read(1)
    while c == '#':
        f.readline()
        c = f.read(1)

    f.seek(-1, 1)
    ret = f.readline()
    return ret

def loadPPM(filename):
    """ Load a PPM image from the specified input file into an Image class instance and rturn it. """
    # read header:
    try:
        f = open(filename, "rb")
    except IOError:
        error("Failed to open file " + filename + ".")
    buf = pnm_read(f)
    if (buf[0:2] != "P6"):
        error("P6 not found in header of file " + filename + ".")

    buf = pnm_read(f).strip().split()
    width = int(buf[0])
    height = int(buf[1])

    buf = pnm_read(f)
    if (int(buf) > MAXVAL):
        error("Max value (third number after P6) in file " + filename + " exceeds maximum allowed value of " + str(MAXVAL) + ".")

    im = Image(width, height, 3)

    # read in pixel values, convert from single bytes to rgb string lists:
    for y in range(height):
        for x in range(width):
            rgb = []
            for i in range(3):
                rgb.append(f.read(1))
            im.set_binary_data(x, y, rgb)

    f.close()
    return im

def savePPM(im, filename):
    width = im.width
    height = im.height

    try:
        f = open(filename, "wb")
    except IOError:
        error("Failed to open file " + filename + ".")

    f.write("P6\n" + str(width) + ' ' + str(height) + '\n' + str(MAXVAL) + '\n')

    for y in range(height):
        for x in range(width):
            rgb = im.get_binary_data(x, y)
            f.write(rgb[0] + rgb[1] + rgb[2])
            
    f.close()
    
# -------------------------------------------------------
# image.h
# -------------------------------------------------------

class Image:
    def __init__(self, width, height, depth):
        self.width = width
        self.height = height
        self.depth = depth # 3 for full image, 1 for segment
        self.data = [[0.0 for value in range(depth)] for value in range(width * height)]

    def set_binary_data(self, x, y, input_data):
        """ Set data at (x, y) from binary-form input of a single element or a list of size self.depth. """
        loc = y * self.width + x
        if self.depth == 1:
            self.data[loc] = struct.unpack('B', input_data)[0]
        else:
            self.data[loc] = [struct.unpack('B', input_data[i])[0] for i in range(self.depth)]

    def set_decimal_data(self, x, y, input_data):
        """ Set data at (x, y) from decimal-form input of a single element or a list of size self.depth. """
        loc = y * self.width + x
        self.data[loc] = input_data

    def get_binary_data(self, x, y):
        """ Return data at (x, y) in binary form as a single element or a list of size self.depth. """
        loc = y * self.width + x
        if self.depth == 1:
            return struct.pack('B', int(self.data[loc]))
        else:
            return [struct.pack('B', int(self.data[loc][i])) for i in range(self.depth)]

    def get_decimal_data(self, x, y):
        """ Return data at (x, y) in decimal form as a single element or a list of size self.depth. """
        loc = y * self.width + x
        if self.depth == 1:
            return self.data[loc]
        else:
            return [self.data[loc][i] for i in range(self.depth)]

# -------------------------------------------------------
# segment-image.h
# -------------------------------------------------------

def random_rgb():
    rgb = []
    for i in range(3):
        rgb.append(random.randrange(0, 256))
    return rgb

def diff(r, g, b, x1, y1, x2, y2):
    return math.sqrt(pow(r.get_decimal_data(x1, y1) - r.get_decimal_data(x2, y2), 2) + \
                     pow(g.get_decimal_data(x1, y1) - g.get_decimal_data(x2, y2), 2) + \
                     pow(b.get_decimal_data(x1, y1) - b.get_decimal_data(x2, y2), 2))

def segment_image(im, sigma, c, min_size):
    """ Segment an image. Returns a color image representing the segmentation and the number of components in the segmentation.
        img: image to segment
        sigma: to smooth the image
        c: constant for threshold function
        min_size: minimum component size (enforced by post-processing stage)"""
    
    width = im.width
    height = im.height

    # separate colors into three channels:
    r = Image(width, height, 1)
    g = Image(width, height, 1)
    b = Image(width, height, 1)
    
    for y in range(height):
        for x in range(width):
            rgb = im.get_decimal_data(x, y)
            r.set_decimal_data(x, y, rgb[0])
            g.set_decimal_data(x, y, rgb[1])
            b.set_decimal_data(x, y, rgb[2])

    # smooth each color channel:
    smooth_r = smooth(r, sigma)
    smooth_g = smooth(g, sigma)
    smooth_b = smooth(b, sigma)

    # build graph:
    edges = []
    num = 0
    for y in range(height):
        for x in range(width):
            if x < width - 1:
                edges.append(Edge(y * width + x, \
                                  y * width + (x + 1), \
                                  diff(smooth_r, smooth_g, smooth_b, x, y, x + 1, y)))
            if y < height - 1:
                edges.append(Edge(y * width + x, \
                                  (y + 1) * width + x, \
                                  diff(smooth_r, smooth_g, smooth_b, x, y, x, y + 1)))
            if x < width - 1 and y < height - 1:
                edges.append(Edge(y * width + x, \
                                  (y + 1) * width + (x + 1), \
                                  diff(smooth_r, smooth_g, smooth_b, x, y, x + 1, y + 1)))
            if x < width - 1 and y > 0:
                edges.append(Edge(y * width + x, \
                                  (y - 1) * width + (x + 1), \
                                  diff(smooth_r, smooth_g, smooth_b, x, y, x + 1, y - 1)))
    while len(edges) < width * height * 4:
        edges.append(Edge(0, 0, 0))
                
    # segment:
    u = segment_graph(width * height, edges, c)

    # post-process small components:
    for i in range(len(edges)):
        a = u.find(edges[i].a)
        b = u.find(edges[i].b)
        if a != b and (u.size(a) < min_size or u.size(b) < min_size):
            u.join(a, b)
    
    num_ccs = u.num_sets()

    output = Image(width, height, 3)

    # pick random colors for each component:
    colors = [random_rgb() for value in range(width * height)]

    for y in range(height):
        for x in range(height):
            comp = u.find(y * width + x)
            output.set_decimal_data(x, y, colors[comp])

    return output, num_ccs
        
# -------------------------------------------------------
# filter.h
# -------------------------------------------------------

def normalize(mask):
    """ Normalize mask so it integrates to one. """
    length = len(mask)
    sum = 0.0
    for i in range(1, length):
        sum = sum + math.fabs(mask[i])
    sum = 2 * sum + math.fabs(mask[0])
    for i in range(length):
        mask[i] = mask[i] / sum
    return mask

def make_fgauss(sigma):
    sigma = max((sigma, 0.01))
    length = int(math.ceil(sigma * WIDTH) + 1)
    mask = [0.0 for value in range(length)]
    for i in range(length):
        mask[i] = math.exp(-0.5 * math.pow(i/sigma, 2))
    return mask

def smooth(src, sigma):
    """ Convolve image with gaussian filter. """
    mask = make_fgauss(sigma)
    mask = normalize(mask)

    tmp = convolve_even(src, mask)
    dst = convolve_even(tmp, mask)

    return dst

# -------------------------------------------------------
# convolve.h
# -------------------------------------------------------

def convolve_even(src, mask):
    """ Convolve src with mask. ret is flipped! """
    width = src.width
    height = src.height
    length = len(mask)
    ret = Image(src.height, src.width, 1)

    for y in range(height):
        for x in range(width):
            sum = mask[0] * src.get_decimal_data(x, y)
            for i in range(1, length):
                sum = sum + mask[i] \
                      * (src.get_decimal_data(max((x - i, 0)), y) \
                      + src.get_decimal_data(min((x + i, width - 1)), y))
            ret.set_decimal_data(y, x, sum)
    return ret

# -------------------------------------------------------
# segment-graph.h
# -------------------------------------------------------

def segment_graph(num_vertices, edges, c):
    """ Segment a graph. Returns a disjoint-set forest representing the segmentation.
        num_vertices: number of vertices in graph
        edges: array of edges
        c: constant for treshold function"""

    # sort edges by weight:
    edges = sorted(edges)

    # make a disjoint-set forest:
    u = Universe(num_vertices)

    # initiate thresholds:
    threshold = [c for value in range(num_vertices)]

    # for each edge, in non-decreasing weight order:
    for i in range(len(edges)):
        # components connected by this edge:
        a = u.find(edges[i].a)
        b = u.find(edges[i].b)
        if a != b:
            if edges[i].w <= threshold[a] and edges[i].w <= threshold[b]:
                u.join(a, b)
                a = u.find(a)
                threshold[a] = edges[i].w + (c / u.size(a))
                
    return u

class Edge:
    def __init__(self, a, b, w):
        self.a = a
        self.b = b
        self.w = w

# -------------------------------------------------------
# disjoint-set.h
# -------------------------------------------------------

class Uni_elt:
    """ Universe element. """
    def __init__(self, rank, size, p):
        self.rank = rank
        self.size = size
        self.p = p
        
class Universe:
    def __init__(self, elements):
        self.elts = [Uni_elt(0, 1, i) for i in range(elements)]
        self.num = elements

    def find(self, x):
        y = x
        while y != self.elts[y].p:
            y = self.elts[y].p
        self.elts[x].p = y
        return y

    def join(self, x, y):
        if self.elts[x].rank > self.elts[y].rank:
            self.elts[y].p = x
            self.elts[x].size = self.elts[x].size + self.elts[y].size
        else:
            self.elts[x].p = y
            self.elts[y].size = self.elts[y].size + self.elts[x].size
            if self.elts[x].rank == self.elts[y].rank:
                self.elts[y].rank = self.elts[y].rank + 1
        self.num = self.num - 1

    def size(self, x):
        return self.elts[x].size

    def num_sets(self):
        return self.num

# -------------------------------------------------------
# segment.cpp (main method)
# -------------------------------------------------------

if (len(sys.argv) != 6):
    sys.stderr.write("usage: " + sys.argv[0] + " sigma k min input(ppm) output(ppm)\n")
    sys.exit()

sigma = float(sys.argv[1])
k = float(sys.argv[2])
min_size = int(sys.argv[3])

print("loading input image.")
input_image = loadPPM(sys.argv[4])

print("processing")
seg, num_ccs = segment_image(input_image, sigma, k, min_size) # return two values
savePPM(seg, sys.argv[5])

print "got", num_ccs, "components"
print "done! uff...thats hard work."
