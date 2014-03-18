#ifndef __PROGTEST__
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <vector>

using namespace std;
#endif

unsigned int pow2(unsigned int a) {
  if (a == 0) { return 1; }
  else { return 2 * (pow2(a-1)); }
}

#define EMPTY_NODE 0
#define CHAR_NODE 1
#define CHAR_SIZE sizeof(char)*8
#define CPT pow2(CHAR_SIZE-1)

#define UTF8_END 4103110784
#define UTF8_INVALID -5

class bitReader {
  unsigned char actualByte;
  int cursor;
  unsigned char * file;
  long size;
  long counter;
  public:

  bitReader (unsigned char _file[], long _size) {
    file = _file;
    size = _size;
    counter = 0;
    actualByte = file[0];
    cursor = CPT*2;
  }

  unsigned int readBit() {
    // Mv the pointer to the next bit
    cursor /= 2;
    if (cursor == 0) {
      counter++;
      if (counter < size) { actualByte = file[counter]; cursor = CPT; }
      else { throw EOF; }
    }
    return (actualByte & cursor) / cursor;
  }

  unsigned int readBits(unsigned int sob) {
    unsigned int _cursor = pow2(sob-1);
    unsigned int res     = 0;
    while (_cursor > 0) {
      unsigned int actualBit = this->readBit();
      res += _cursor * actualBit;
      _cursor /= 2;
    }

    return res;
  }
};

class hTree {
  struct hNode {
    unsigned int val;
    bool leaf;
    hNode * left, * right;
  };

  hNode * root;

  bool vb (unsigned int nb) {
    if ((nb & 192) == 128 ) { return true; }
    return false;
  }

  unsigned int readUTF8 (bitReader &file) {

    unsigned int b1 = file.readBits(8);
    if ((b1 & 128) == 0) { return b1; }
    else if ((b1 & 224) == 192) {
      unsigned int b2 = file.readBits(8);
      if (vb(b2)) { return (b1 * 256 + b2); }
    }
    else if ((b1 & 240) == 224) {
      unsigned int b2 = file.readBits(8);
      unsigned int b3 = file.readBits(8);
      if (vb(b2) && vb(b3)) {
        return ( b1 * 65536 + b2 * 256 + b3 );
      }
    }
    else if ((b1 & 248) == 240) {
      unsigned int b2 = file.readBits(8);
      unsigned int b3 = file.readBits(8);
      unsigned int b4 = file.readBits(8);
      unsigned int nb = b1 * 16777216 + b2 * 65536 + b3 * 256 + b4;
      if (nb == UTF8_END || (vb(b2) && vb(b3) && vb(b4))) {
        return nb;
      }
    }

    throw UTF8_INVALID;
  }

  hNode * constructor (bitReader &file, bool nend) {
    unsigned int aBit = file.readBit();
    hNode * n = new hNode;
    if (aBit == 0) {
      n->val   = 0;
      n->leaf  = false;
      n->left  = constructor(file, nend);
      if (nend) { n->right = constructor(file, nend); }
      else { n->right = NULL; }
    } else {
      n->leaf  = true;
      n->val   = readUTF8(file);
      n->left  = NULL;
      n->right = NULL;
      if (n->val == UTF8_END) { nend = false; }
    }

    return n;
  }

  void destructor (hNode * n) {
    if ( n->left )  { destructor(n->left); }
    if ( n->right ) { destructor(n->right); }

    delete n;
  }

  public:

  hTree (bitReader &file) {
    bool nend = true;
    root = constructor (file, nend);
  }

  ~hTree () {
    if (root)
      destructor (root);
  }

  vector <unsigned char> eval(bitReader &file) {
    hNode * n = root;
    unsigned int curs = file.readBit();
    vector <unsigned char> res;

    while (true) {
      if (curs == 0) {
        n = n->left;
      } else {
        n = n->right;
      }

      if (n->leaf) {
        if (n->val != UTF8_END) {
          unsigned int nVal = n->val;
          if (nVal < 256) { res.push_back( (char) nVal );}
          else if (nVal < 65536) {
            res.push_back( (char) (nVal/256) );
            res.push_back( (char) (nVal%256) );
          } else if (nVal < 16777216) {
            res.push_back( (char) (nVal/65536) );
            res.push_back( (char) ( (nVal%65536)/256) );
            res.push_back( (char) ( (nVal%65536)%256) );
          } else {
            res.push_back( (char) (nVal/16777216) );
            res.push_back( (char) ( (nVal%16777216)/65536) );
            res.push_back( (char) ( ((nVal%16777216)%65536)/256) );
            res.push_back( (char) ( ((nVal%16777216)%65536)%256) );
          }
          n = root;
        } else { break; }
      }

      if (n == NULL || n->left == NULL || n->right == NULL) {
        cout << "WTF!" << endl;
        throw -9;
      }

      try {
        curs = file.readBit();
      } catch (int e) {
        if (e == EOF) { throw EOF; }
        else { cout << "Error: " << e << endl; break; }
      }
    }

    return res;
  }
};

bool compressFile ( const char * inFile, const char * outFile ) {
  return false;
}

bool decompressFile ( const char * inFile, const char * outFile ) {

  FILE * input  = fopen(inFile , "rb");
  if (input == NULL) {
    return false;
  }

  fseek(input, 0, SEEK_END);
  long fsize = ftell(input);
  fseek(input, 0, SEEK_SET);
  unsigned char * allFile = new unsigned char [fsize];
  fread(allFile, fsize, 1, input);
  fclose (input);
  bitReader inputBits (allFile, fsize);

  hTree * huffTree = new hTree (inputBits);

  vector <unsigned char> out;
  try { out = huffTree->eval(inputBits); }
  catch (int e) { return false; }

  FILE * output = fopen(outFile, "wb");
  if (input == NULL) {
    return false;
  }

  fwrite (&out[0], sizeof(unsigned char), out.size(), output);
  fclose(output);

  delete huffTree;
  delete [] allFile;

  return true;
}

#ifndef __PROGTEST__
int main ( int argc, char * argv [] ) {

  decompressFile(argv[1],argv[2]);

  return 0;
}
#endif
