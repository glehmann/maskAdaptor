#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCommand.h"
#include <itkBinaryThresholdImageFilter.h>

#include "itkOtsuMultipleThresholdsImageFilter.h"
#include "itkMaskAdaptorImageFilter.h"

int main(int, char * argv[])
{
  const int dim = 2;
  
  typedef unsigned char PType;
  typedef itk::Image< PType, dim > IType;

  typedef itk::ImageFileReader< IType > ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( argv[1] );

  typedef itk::BinaryThresholdImageFilter<IType, IType> ThreshType;
  ThreshType::Pointer thresh = ThreshType::New();
  thresh->SetInput(reader->GetOutput());
  thresh->SetUpperThreshold(50);
  thresh->SetInsideValue(1);
  thresh->SetOutsideValue(0);

  typedef itk::OtsuMultipleThresholdsImageFilter<IType, IType> OtsuType;
  OtsuType::Pointer filter = OtsuType::New();
  filter->SetInput(reader->GetOutput());
  filter->SetNumberOfThresholds(2);

  typedef itk::ImageFileWriter< IType > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetInput( filter->GetOutput() );
  writer->SetFileName( argv[2] );
  writer->Update();

  // now for the masked version
  typedef itk::MaskAdaptorImageFilter<IType, IType, IType> MaskAdaptorType;
  typedef itk::OtsuMultipleThresholdsImageFilter<MaskAdaptorType::InternalInputImageType, MaskAdaptorType::InternalOutputImageType> MaskOtsuType;

  MaskAdaptorType::Pointer masker = MaskAdaptorType::New();
  MaskOtsuType::Pointer maskotsu = MaskOtsuType::New();
  maskotsu->SetNumberOfThresholds(1);
  masker->SetFilter(maskotsu);
  masker->SetInput(reader->GetOutput());
  masker->SetMaskImage(thresh->GetOutput());
  masker->SetDefaultValue(0);
  writer->SetInput( masker->GetOutput() );
  writer->SetFileName( argv[3] );
  writer->Update();

  return 0;
}

