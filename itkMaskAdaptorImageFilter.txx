#ifndef __itkMaskAdaptorImageFilter_txx
#define __itkMaskAdaptorImageFilter_txx

#include "itkMaskAdaptorImageFilter.h"
#include "itkProgressReporter.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIterator.h"
#include "itkNumericTraits.h"

namespace itk {

template <class TInputImage, class TMaskImage, class TOutputImage, class TInputFilter, class TOutputFilter, class TInternalInputImageType, class TInternalOutputImageType>
MaskAdaptorImageFilter<TInputImage, TMaskImage, TOutputImage, TInputFilter, TOutputFilter, TInternalInputImageType, TInternalOutputImageType>
::MaskAdaptorImageFilter()
{
  m_InputFilter = NULL;
  m_OutputFilter = NULL;
  m_DefaultValue = NumericTraits<OutputPixelType>::Zero;
  m_PassOutsideMask = false;

}


template <class TInputImage, class TMaskImage, class TOutputImage, class TInputFilter, class TOutputFilter, class TInternalInputImageType, class TInternalOutputImageType>
void
MaskAdaptorImageFilter<TInputImage, TMaskImage, TOutputImage, TInputFilter, TOutputFilter, TInternalInputImageType, TInternalOutputImageType>
::GenerateInputRequestedRegion()
{
  // call the superclass' implementation of this method
  Superclass::GenerateInputRequestedRegion();

  typename InputImageType::Pointer  inputPtr =
    const_cast< InputImageType * >( this->GetInput( 0 ) );
  
  if ( !inputPtr )
    { return; }
  
  inputPtr->SetRequestedRegion(inputPtr->GetLargestPossibleRegion());

  typename MaskImageType::Pointer  maskPtr =
    const_cast< MaskImageType * >( this->GetMaskImage() );
  
  if ( !maskPtr )
    { return; }
  
  maskPtr->SetRequestedRegion(maskPtr->GetLargestPossibleRegion());

}


template <class TInputImage, class TMaskImage, class TOutputImage, class TInputFilter, class TOutputFilter, class TInternalInputImageType, class TInternalOutputImageType>
void
MaskAdaptorImageFilter<TInputImage, TMaskImage, TOutputImage, TInputFilter, TOutputFilter, TInternalInputImageType, TInternalOutputImageType>
::EnlargeOutputRequestedRegion(DataObject *)
{
  for( unsigned i=0; i<this->GetNumberOfOutputs(); i++)
    {
    this->GetOutput( i )->SetRequestedRegion( this->GetOutput( i )->GetLargestPossibleRegion() );
    }
}


template <class TInputImage, class TMaskImage, class TOutputImage, class TInputFilter, class TOutputFilter, class TInternalInputImageType, class TInternalOutputImageType>
void
MaskAdaptorImageFilter<TInputImage, TMaskImage, TOutputImage, TInputFilter, TOutputFilter, TInternalInputImageType, TInternalOutputImageType>
::SetFilter( InputFilterType * filter )
{
  OutputFilterType * outputFilter = dynamic_cast< OutputFilterType * >( filter );
  if( outputFilter == NULL && filter != NULL )
    {
    // TODO: can it be replaced by a concept check ?
    itkExceptionMacro("Wrong output filter type. Use SetOutputFilter() and SetInputFilter() instead of SetFilter() when input and output filter types are different.");
    }
  this->SetInputFilter( filter );
  this->SetOutputFilter( outputFilter );
}


template <class TInputImage, class TMaskImage, class TOutputImage, class TInputFilter, class TOutputFilter, class TInternalInputImageType, class TInternalOutputImageType>
void
MaskAdaptorImageFilter<TInputImage, TMaskImage, TOutputImage, TInputFilter, TOutputFilter, TInternalInputImageType, TInternalOutputImageType>
::SetInputFilter( InputFilterType * filter )
{
  if( m_InputFilter.GetPointer() != filter )
    {
    this->Modified();
    m_InputFilter = filter;
    // adapt the number of inputs and outputs
    this->SetNumberOfRequiredInputs( filter->GetNumberOfValidRequiredInputs() );
    }
}


template <class TInputImage, class TMaskImage, class TOutputImage, class TInputFilter, class TOutputFilter, class TInternalInputImageType, class TInternalOutputImageType>
void
MaskAdaptorImageFilter<TInputImage, TMaskImage, TOutputImage, TInputFilter, TOutputFilter, TInternalInputImageType, TInternalOutputImageType>
::SetOutputFilter( OutputFilterType * filter )
{
  if( m_OutputFilter.GetPointer() != filter )
    {
    this->Modified();
    m_OutputFilter = filter;
    // adapt the number of inputs and outputs
    this->SetNumberOfRequiredOutputs( filter->GetNumberOfOutputs() );
    }
}


template <class TInputImage, class TMaskImage, class TOutputImage, class TInputFilter, class TOutputFilter, class TInternalInputImageType, class TInternalOutputImageType>
void
MaskAdaptorImageFilter<TInputImage, TMaskImage, TOutputImage, TInputFilter, TOutputFilter, TInternalInputImageType, TInternalOutputImageType>
::GenerateData()
{
  if( !m_InputFilter || !m_OutputFilter )
    {
    itkExceptionMacro("Filters must be set.");
    }

  for( unsigned i=1; i<this->GetNumberOfInputs(); i++)
    {
    if ( this->GetInput()->GetRequestedRegion().GetSize() != this->GetInput( i )->GetRequestedRegion().GetSize() )
      {
      itkExceptionMacro( << "Inputs must have the same size." );
      }
    }

  this->AllocateOutputs();

  RegionType requestedRegion = this->GetOutput()->GetRequestedRegion();
  IndexType requestedIndex = requestedRegion.GetIndex();
  SizeType requestedSize = requestedRegion.GetSize();

  // need several passes over the mask image. The first pass counts
  // the number of on pixels. We then allocate a 1D image to contain
  // them all, and the iterate over both input and mask image to copy
  // the relevant pixels in
  typedef typename itk::ImageRegionConstIterator<TMaskImage> MaskIterType;
  typedef typename itk::ImageRegionConstIterator<TInputImage> InputIterType;
  typedef typename itk::ImageRegionIterator<InternalInputImageType> OneDIterType;
  typedef typename itk::ImageRegionIterator<InternalOutputImageType> OneDOutIterType;

  const MaskImageType *maskPtr = this->GetMaskImage();
  const InputImageType *inputPtr = this->GetInput();
  OutputImageType *outputPtr = this->GetOutput();

  unsigned long maskcount = 0;
  MaskIterType maskIt(maskPtr, requestedRegion);
  for (maskIt.GoToBegin(); !maskIt.IsAtEnd(); ++maskIt)
    {
    if (maskIt.Get()) 
      {
      ++maskcount;
      }
    }


  typename InternalInputImageType::Pointer Buffer1D = InternalInputImageType::New();
  typename InternalInputImageType::RegionType reg1D;
  typename InternalInputImageType::SizeType size1D;
  size1D.Fill(maskcount);
  reg1D.SetSize(size1D);
  Buffer1D->SetRegions(reg1D);
  Buffer1D->Allocate();

  InputIterType inIt(inputPtr, requestedRegion);
  OneDIterType oneDIt(Buffer1D, Buffer1D->GetLargestPossibleRegion());

  for (maskIt.GoToBegin(), inIt.GoToBegin(), oneDIt.GoToBegin(); 
       !maskIt.IsAtEnd(); ++maskIt, ++inIt)
    {
    if (maskIt.Get()) 
      {
      oneDIt.Set(inIt.Get());
      ++oneDIt;
      }
    }
  
  m_InputFilter->SetInput(Buffer1D);
  typename InternalOutputImageType::Pointer result = m_OutputFilter->GetOutput();
  result->Update();
  result->DisconnectPipeline();
  // now copy the result to the output image
  typedef typename itk::ImageRegionIterator<OutputImageType> OutputIterType;
  OutputIterType outIt(outputPtr, requestedRegion);
  OneDOutIterType resultIt(result, result->GetLargestPossibleRegion());

  if (m_PassOutsideMask)
    {
    for (maskIt.GoToBegin(), outIt.GoToBegin(), inIt.GoToBegin(), resultIt.GoToBegin(); 
	 !maskIt.IsAtEnd(); ++maskIt, ++outIt, ++inIt)
      {
      if (maskIt.Get()) 
	{
	outIt.Set(resultIt.Get());
	++resultIt;
	}
      else
	{
	outIt.Set(static_cast<OutputPixelType>(inIt.Get()));
	}
      }
    }
  else
    {
    for (maskIt.GoToBegin(), outIt.GoToBegin(), resultIt.GoToBegin(); 
	 !maskIt.IsAtEnd(); ++maskIt, ++outIt)
      {
      if (maskIt.Get()) 
	{
	outIt.Set(resultIt.Get());
	++resultIt;
	}
      else
	{
	outIt.Set(m_DefaultValue);
	}
      }
    }  
}


template <class TInputImage, class TMaskImage, class TOutputImage, class TInputFilter, class TOutputFilter, class TInternalInputImageType, class TInternalOutputImageType>
void
MaskAdaptorImageFilter<TInputImage, TMaskImage, TOutputImage, TInputFilter, TOutputFilter, TInternalInputImageType, TInternalOutputImageType>
::PrintSelf(std::ostream &os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

}

}


#endif
