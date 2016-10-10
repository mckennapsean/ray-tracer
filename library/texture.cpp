// namespace
using namespace scene;


// Texture from a file
class TextureFile: public Texture{
  private:
    
    // variables
    vector<Color24> data;
    int width;
    int height;
    
    // load a PPM image file into the texture data
    bool loadPPM(string file){
      
      // first load in file (binary, too)
      ifstream f(file, ios::in | ios::binary);
      
      // begin reading header into a string
      string s;
      getline(f, s);
      
      // ensure we have a proper PPM file
      if(s.at(0) != 'P' && s.at(1) != '6')
        return false;
      
      // ignore comments in header
      getline(f, s);
      while(s.at(0) == '#')
        getline(f, s);
      
      // grab dimensions of PPM
      stringstream ss(s);
      ss >> width >> height;
      
      // continue ignoring comments
      getline(f, s);
      while(s.at(0) == '#')
        getline(f, s);
      
      // load in data for PPM image
      int t = width * height;
      data.resize(t);
      f.read((char*) data.data(), t * sizeof(Color24));
      
      // close stream
      f.close();
      
      // return a successful load from file
      return true;
    }
  
  public:
    
    // constructor
    TextureFile(){
      width = 0;
      height = 0;
    }
    
    // load a texture from a file
    bool load(){
      
      // reset data contents
      data.clear();
      width = 0;
      height = 0;
      
      // load file, if we have a succesful PPM image file
      return loadPPM(getName());
    }
    
    // sample a texture for a color
    Color sample(Point &uvw){
      
      // empty texture
      if(width + height == 0)
        return Color(0.0, 0.0, 0.0);
      
      // clamp position into texture space
      
      // calculate sampling positions
      Point u = tileClamp(uvw);
      float x = width * u.x;
      float y = height * u.y;
      int ix = (int) x;
      int iy = (int) y;
      float fx = x - ix;
      float fy = y - iy;
      
      // test near edges to shift sampling positions
      if(ix < 0)
        ix -= (ix / width - 1.0) * width;
      if(ix >= width)
        ix -= (ix / width) * width;
      int ixp = ix + 1;
      if(ixp >= width)
        ixp -= width;
      if(iy < 0)
        iy -= (iy / height - 1.0) * height;
      if(iy >= height)
        iy -= (iy / height) * height;
      int iyp = iy + 1;
      if(iyp >= height)
        iyp -= height;
      
      // return the color at this point
      return data[iy * width + ix].ToColor() * ((1.0 - fx) * (1.0 - fy)) + data[iy * width + ixp].ToColor() * (fx * ( 1.0 - fy)) + data[iyp * width + ix].ToColor() * ((1.0 - fx) * fy) + data[iyp * width + ixp].ToColor() * (fx * fy);
    }
};


// Procedural texture (checkerboard)
class TextureChecker: public Texture{
  private:
    
    // variables
    Color color1;
    Color color2;
    
  public:
    
    // constructor
    TextureChecker(){
      color1 = Color(0.0, 0.0, 0.0);
      color2 = Color(1.0, 1.0, 1.0);
    }
    
    // set texture checker colors
    void setColor1(Color c){
      color1 = c;
    }
    void setColor2(Color c){
      color2 = c;
    }
    
    // sample a texture for a color
    Color sample(Point &uvw){
      
      // clamp position into texture space
      Point u = tileClamp(uvw);
      
      // one of four cases to one of two colors, in a checkerboard fashion
      if(u.x <= 0.5){
        if(u.y <= 0.5)
          return color1;
        else
          return color2;
      }else{
        if(u.y <= 0.5)
          return color2;
        else
          return color1;
      }
    }
};
