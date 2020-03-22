#ifndef _INC_WIMAGEHLP_
#define _INC_WIMAGEHLP_

#ifndef _IMAGEHLP_
#  include <imagehlp.h>
#endif

class WMappedImage : public LOADED_IMAGE
{
   protected:
   bool bGood;

   public:
   operator bool() const
   {
      return bGood;
   }

   WMappedImage(LPSTR ImageName, LPSTR DllPath, BOOL DotDll, BOOL ReadOnly)
   {
      ZeroMemory(this, sizeof *this);
      bGood = MapAndLoad(ImageName, DllPath, this, DotDll, ReadOnly) == TRUE;
   }

   ~WMappedImage()
   {
      if (bGood)
         UnMapAndLoad(this);
   }
};

#endif // _INC_WIMAGEHLP_

