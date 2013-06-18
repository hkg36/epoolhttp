struct GeoFun{
  static unsigned long long CombineForGeo(float lat,float lng){
    return Combine((unsigned int)((lat+90)*1e6),(unsigned int)((lng+180)*1e6)); 
  }
  static void SplitForGeo(unsigned long long src,float &lat,float &lng){
    unsigned int latint,lngint;
    Split(src,&latint,&lngint);
    lat=((float)latint)/1e6 - 90;
    lng=((float)lngint)/1e6 - 180;
  }
  static unsigned long long Combine(unsigned int first,unsigned second);
  static void Split(unsigned long long src,unsigned int *first,unsigned int *second);
  static void LongToByte(unsigned long long src,unsigned char des[8]);
  static unsigned long long ByteToLong(unsigned char src[8]);
};