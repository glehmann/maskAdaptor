#ifndef __itkMaskAdaptorImageFilter_h
#define __itkMaskAdaptorImageFilter_h

#include "itkImageToImageFilter.h"
#include "itkCastImageFilter.h"


namespace itk {

/**
 * \class MaskAdaptorImageFilter
 * \brief Extract a region defined by a mask, apply a pipeline
 * processing to it and place results back in image. Intended for
 * operations like Otsu thresholding applied to a restricted
 * region. Obviously no use for neighborhood operations.
 *
 *
 * \author Richard Beare
 */

template<class TInputImage,
	 class TMaskImage,
	 class TOutputImage,
	 class TInputFilter=ImageToImageFilter< Image< typename TInputImage::PixelType, 1 >,  Image< typename TOutputImage::PixelType, 1 > >,
	 class TOutputFilter=TInputFilter,
	 class TInternalInputImage=typename TInputFilter::InputImageType,
	 class TInternalOutputImage=typename TOutputFilter::OutputImageType >
class ITK_EXPORT MaskAdaptorImageFilter : 
public ImageToImageFilter<TInputImage, TOutputImage>
{
public:
  /** Standard class typedefs. */
  typedef MaskAdaptorImageFilter Self;
  typedef ImageToImageFilter<TInputImage,TOutputImage>  Superclass;
  typedef SmartPointer<Self>        Pointer;
  typedef SmartPointer<const Self>  ConstPointer;
  
  /** Standard New method. */
  itkNewMacro(Self);

  /** Runtime information support. */
  itkTypeMacro(MaskAdaptorImageFilter,
               ImageToImageFilter);
 
  /** Image related typedefs. */
  typedef TInputImage InputImageType;
  typedef typename TInputImage::RegionType RegionType ;
  typedef typename TInputImage::SizeType SizeType ;
  typedef typename TInputImage::IndexType IndexType ;
  typedef typename TInputImage::PixelType PixelType ;
  typedef typename TInputImage::OffsetType OffsetType ;

  typedef TMaskImage MaskImageType;
  typedef typename TMaskImage::RegionType MaskRegionType ;
  typedef typename TMaskImage::SizeType MaskSizeType ;
  typedef typename TMaskImage::IndexType MaskIndexType ;
  typedef typename TMaskImage::PixelType MaskPixelType ;
  typedef typename TMaskImage::OffsetType MaskOffsetType ;
  
  typedef TOutputImage OutputImageType;
  typedef typename TOutputImage::PixelType OutputPixelType ;

  typedef TInputFilter  InputFilterType ;
  typedef TOutputFilter OutputFilterType ;
  
  typedef TInternalInputImage InternalInputImageType;
  typedef typename InternalInputImageType::RegionType InternalRegionType ;
  typedef typename InternalInputImageType::SizeType InternalSizeType ;
  typedef typename InternalInputImageType::IndexType InternalIndexType ;
  typedef typename InternalInputImageType::OffsetType InternalOffsetType ;
  typedef typename InternalInputImageType::PixelType InternalInputPixelType ;

  typedef TInternalOutputImage InternalOutputImageType;
  typedef typename InternalOutputImageType::PixelType InternalOutputPixelType ;

  /** Image related typedefs. */
  itkStaticConstMacro(ImageDimension, unsigned int,
                      TInputImage::ImageDimension);

  itkStaticConstMacro(InternalImageDimension, unsigned int,
                      InternalInputImageType::ImageDimension);

  // itkSetObjectMacro(Filter, FilterType);
  void SetFilter(InputFilterType * filter);
  InputFilterType * GetFilter()
    {
    return m_InputFilter;
    }

  const InputFilterType * GetFilter() const
    {
    return m_InputFilter;
    }

   /** Set the mask image */
  void SetMaskImage(TMaskImage *input)
     {
       // Process object is not const-correct so the const casting is required.
       this->SetNthInput( 1, const_cast<TMaskImage *>(input) );
     }
  /** Get the mask image */
  MaskImageType * GetMaskImage()
    {
      return static_cast<MaskImageType*>(const_cast<DataObject *>(this->ProcessObject::GetInput(1)));
    }

  void SetInputFilter(InputFilterType * filter);
  itkGetObjectMacro(InputFilter, InputFilterType);

  void SetOutputFilter(OutputFilterType * filter);
  itkGetObjectMacro(OutputFilter, OutputFilterType);

  void GenerateInputRequestedRegion() ;
  void EnlargeOutputRequestedRegion(DataObject *itkNotUsed(output));

  itkSetMacro(DefaultValue, OutputPixelType);
  itkGetMacro(DefaultValue, OutputPixelType);

protected:
  MaskAdaptorImageFilter();
  ~MaskAdaptorImageFilter() {};

  void GenerateData();

  void PrintSelf(std::ostream& os, Indent indent) const;

  typename InputFilterType::Pointer m_InputFilter;
  typename OutputFilterType::Pointer m_OutputFilter;

private:
  MaskAdaptorImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
  OutputPixelType m_DefaultValue;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMaskAdaptorImageFilter.txx"
#endif

#endif
