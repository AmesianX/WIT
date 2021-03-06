/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkFilledCaptionActor2D.cpp,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

// Defines vtk data types based on 32/64 bit flag.
#include "vtkConfigure.h"

#include "vtkFilledCaptionActor2D.h"

#include "vtkActor.h"
#include "vtkAppendPolyData.h"
#include "vtkCellArray.h"
#include "vtkDoubleArray.h"
#include "vtkGlyph2D.h"
#include "vtkGlyph3D.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkProperty.h"
#include "vtkProperty2D.h"
#include "vtkTextActor.h"
#include "vtkTextProperty.h"
#include "vtkViewport.h"

vtkCxxRevisionMacro(vtkFilledCaptionActor2D, "$Revision: 1.1.1.1 $");
vtkStandardNewMacro(vtkFilledCaptionActor2D);

vtkCxxSetObjectMacro(vtkFilledCaptionActor2D,LeaderGlyph,vtkPolyData);
vtkCxxSetObjectMacro(vtkFilledCaptionActor2D,CaptionTextProperty,vtkTextProperty);

//----------------------------------------------------------------------------
vtkFilledCaptionActor2D::vtkFilledCaptionActor2D()
{
     this->mStaticOffset = 27;
  // Positioning information
  this->AttachmentPointCoordinate = vtkCoordinate::New();
  this->AttachmentPointCoordinate->SetCoordinateSystemToWorld();
  this->AttachmentPointCoordinate->SetValue(0.0,0.0,0.0);

  this->PositionCoordinate->SetCoordinateSystemToDisplay();
  this->PositionCoordinate->SetReferenceCoordinate(
    this->AttachmentPointCoordinate);
  this->PositionCoordinate->SetValue(static_cast<double>(10),
                                     static_cast<double>(10));
  this->Position2Coordinate->SetCoordinateSystemToDisplay();
  
  // This sets up the Position2Coordinate
  this->vtkActor2D::SetWidth(0.25);
  this->vtkActor2D::SetHeight(0.10);

  this->Caption = NULL;
  this->Border = 1;
  this->Leader = 1;
  this->ThreeDimensionalLeader = 1;
  this->LeaderGlyphSize = 0.025;
  this->MaximumLeaderGlyphSize = 20;
  this->LeaderGlyph = NULL;
  
  // Control font properties
  this->Padding = 3;

  this->CaptionTextProperty = vtkTextProperty::New();
  this->CaptionTextProperty->SetBold(0);
  this->CaptionTextProperty->SetItalic(0);
  this->CaptionTextProperty->SetShadow(0);
  this->CaptionTextProperty->SetFontFamily(VTK_ARIAL);
  this->CaptionTextProperty->SetJustification(VTK_TEXT_LEFT);
  this->CaptionTextProperty->SetVerticalJustification(VTK_TEXT_CENTERED);
  this->CaptionTextProperty->SetFontSize(8);

  // What is actually drawn
  this->CaptionActor = vtkTextActor::New();
  this->CaptionActor->GetPositionCoordinate()->SetCoordinateSystemToDisplay();
  this->CaptionActor->GetPositionCoordinate()->SetReferenceCoordinate(NULL);
  this->CaptionActor->GetPosition2Coordinate()->SetCoordinateSystemToDisplay();
  this->CaptionActor->GetPosition2Coordinate()->SetReferenceCoordinate(NULL);
#if VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION >= 2
  this->CaptionActor->SetTextScaleMode(vtkTextActor::TEXT_SCALE_MODE_PROP);
#else
  this->CaptionActor->SetScaledText(1);
#endif

  this->BorderPolyData = vtkPolyData::New();
  this->BorderFillPolyData = vtkPolyData::New();
  vtkPoints *pts = vtkPoints::New();
  pts->SetNumberOfPoints(4);
  this->BorderPolyData->SetPoints(pts);
  this->BorderFillPolyData->SetPoints(pts);
  pts->Delete();
  vtkCellArray *border = vtkCellArray::New();
  border->InsertNextCell(5);
  border->InsertCellPoint(0);
  border->InsertCellPoint(1);
  border->InsertCellPoint(2);
  border->InsertCellPoint(3);
  border->InsertCellPoint(0);
  this->BorderPolyData->SetLines(border);
  this->BorderFillPolyData->SetPolys(border);
  border->Delete();
  
  this->BorderMapper = vtkPolyDataMapper2D::New();
  this->BorderMapper->SetInput(this->BorderPolyData);
  this->BorderActor = vtkActor2D::New();
  this->BorderActor->SetMapper(this->BorderMapper);

  this->BorderFillMapper = vtkPolyDataMapper2D::New();
  this->BorderFillMapper->SetInput(this->BorderFillPolyData);
  this->BorderFillActor = vtkActor2D::New();
  this->BorderFillActor->SetMapper(this->BorderFillMapper);
  //this->BorderActor->GetProperty()->SetColor(0.5,0.5,0.5);

  // This is for glyphing the head of the leader
  // A single point with a vector for glyph orientation
  this->HeadPolyData = vtkPolyData::New();
  pts = vtkPoints::New();
  pts->SetNumberOfPoints(1);
  this->HeadPolyData->SetPoints(pts);
  pts->Delete();
  vtkDoubleArray *vecs = vtkDoubleArray::New();
  vecs->SetNumberOfComponents(3);
  vecs->SetNumberOfTuples(1);
  this->HeadPolyData->GetPointData()->SetVectors(vecs);
  vecs->Delete();

  // This is the leader (line) from the attachment point to the caption
  this->LeaderPolyData = vtkPolyData::New();
  pts = vtkPoints::New();
  pts->SetNumberOfPoints(2);
  this->LeaderPolyData->SetPoints(pts);
  pts->Delete();
  vtkCellArray *leader = vtkCellArray::New();
  leader->InsertNextCell(2); 
  leader->InsertCellPoint(0);
  leader->InsertCellPoint(1); //at the attachment point
  this->LeaderPolyData->SetLines(leader);
  leader->Delete();

  // Used to generate the glyph on the leader head
  this->HeadGlyph = vtkGlyph3D::New();
  this->HeadGlyph->SetInput(this->HeadPolyData);
  this->HeadGlyph->SetScaleModeToDataScalingOff();
  this->HeadGlyph->SetScaleFactor(0.1);

  // Appends the leader and the glyph head
  this->AppendLeader = vtkAppendPolyData::New();
  this->AppendLeader->UserManagedInputsOn();
  this->AppendLeader->SetNumberOfInputs(2);
  this->AppendLeader->SetInputByNumber(0,this->LeaderPolyData);
  this->AppendLeader->SetInputByNumber(1,this->HeadGlyph->GetOutput());

  // Used to transform from world to other coordinate systems
  this->MapperCoordinate2D = vtkCoordinate::New();
  this->MapperCoordinate2D->SetCoordinateSystemToWorld();

  // If 2D leader is used, then use this mapper/actor combination
  this->LeaderMapper2D = vtkPolyDataMapper2D::New();
  this->LeaderMapper2D->SetTransformCoordinate(this->MapperCoordinate2D);
  this->LeaderActor2D = vtkActor2D::New();
  this->LeaderActor2D->SetMapper(this->LeaderMapper2D);

  // If 3D leader is used, then use this mapper/actor combination
  this->LeaderMapper3D = vtkPolyDataMapper::New();
  this->LeaderActor3D = vtkActor::New();
  this->LeaderActor3D->SetMapper(this->LeaderMapper3D);
  SetMapper(vtkPolyDataMapper2D::New());
}

//----------------------------------------------------------------------------
vtkFilledCaptionActor2D::~vtkFilledCaptionActor2D()
{
  if ( this->Caption )
    {
    delete [] this->Caption;
    }
  
  this->AttachmentPointCoordinate->Delete();

  this->CaptionActor->Delete();
  
  if ( this->LeaderGlyph )
    {
    this->LeaderGlyph->Delete();
    }

  this->BorderPolyData->Delete();
  this->BorderMapper->Delete();
  this->BorderActor->Delete();

  this->BorderFillPolyData->Delete();
  this->BorderFillMapper->Delete();
  this->BorderFillActor->Delete();

  this->HeadPolyData->Delete();
  this->LeaderPolyData->Delete();
  this->HeadGlyph->Delete();
  this->AppendLeader->Delete();
  
  this->MapperCoordinate2D->Delete();

  this->LeaderMapper2D->Delete();
  this->LeaderActor2D->Delete();

  this->LeaderMapper3D->Delete();
  this->LeaderActor3D->Delete();

  this->SetCaptionTextProperty(NULL);
}

//----------------------------------------------------------------------------
// Release any graphics resources that are being consumed by this actor.
// The parameter window could be used to determine which graphic
// resources to release.
void vtkFilledCaptionActor2D::ReleaseGraphicsResources(vtkWindow *win)
{
  this->CaptionActor->ReleaseGraphicsResources(win); 
  this->BorderActor->ReleaseGraphicsResources(win); 
  this->BorderFillActor->ReleaseGraphicsResources(win); 
  this->LeaderActor2D->ReleaseGraphicsResources(win); 
  this->LeaderActor3D->ReleaseGraphicsResources(win); 
}

int vtkFilledCaptionActor2D::RenderOverlay(vtkViewport *viewport)
{
  int renderedSomething = 0;

  

  if ( this->Border )
    {
		// dla
		this->BorderFillActor->GetProperty()->SetColor(0.5,0.5,0.5);
		renderedSomething += this->BorderFillActor->RenderOverlay(viewport);
		// dla
		this->BorderActor->GetProperty()->SetColor(1,1,1);
		renderedSomething += this->BorderActor->RenderOverlay(viewport);
    }

  if ( this->Leader )
    {
    if ( this->ThreeDimensionalLeader )
      {
      renderedSomething += this->LeaderActor3D->RenderOverlay(viewport);
      }
    else
      {
      renderedSomething += this->LeaderActor2D->RenderOverlay(viewport);
      }
    }

	renderedSomething += this->CaptionActor->RenderOverlay(viewport);

  return renderedSomething;
}

//----------------------------------------------------------------------------
int vtkFilledCaptionActor2D::RenderOpaqueGeometry(vtkViewport *viewport)
{
  // Build the caption (almost always needed so we don't check mtime)
  vtkDebugMacro(<<"Rebuilding caption");

  // compute coordinates and set point values
  //
  double *w1, *w2;
  int *x1, *x2, *x3;
  double p1[4], p2[4], p3[4];
  x1 = this->AttachmentPointCoordinate->GetComputedDisplayValue(viewport);
  x2 = this->PositionCoordinate->GetComputedDisplayValue(viewport);
  x3 = this->Position2Coordinate->GetComputedDisplayValue(viewport);
  p1[0] = (double)x1[0]; p1[1] = (double)x1[1]; p1[2] = 0.0;
  p2[0] = (double)x2[0]; p2[1] = (double)x2[1]; p2[2] = p1[2];
  //p3[0] = (double)x3[0]; p3[1] = (double)x3[1]; p3[2] = p1[2];
  p3[0] = p2[0]+mStaticOffset; p3[1] = p2[1]+35; p3[2] = p1[2];

  // Set up the scaled text - take into account the padding
   this->CaptionActor->SetTextProperty(this->CaptionTextProperty);
   this->CaptionActor->GetPositionCoordinate()->SetValue(
     p2[0]+this->Padding,p2[1]+this->Padding,0.0);
   this->CaptionActor->GetPosition2Coordinate()->SetValue(
     p3[0]-this->Padding,p3[1]-this->Padding,0.0);

  // Define the border
  vtkPoints *pts = this->BorderPolyData->GetPoints();
  pts->SetPoint(0, p2);
  pts->SetPoint(1, p3[0],p2[1],p1[2]);
  pts->SetPoint(2, p3[0],p3[1],p1[2]);
  pts->SetPoint(3, p2[0],p3[1],p1[2]);

  // Define the border
  pts = this->BorderFillPolyData->GetPoints();
  pts->SetPoint(0, p2);
  pts->SetPoint(1, p3[0],p2[1],p1[2]);
  pts->SetPoint(2, p3[0],p3[1],p1[2]);
  pts->SetPoint(3, p2[0],p3[1],p1[2]);

  // Define the leader. Have to find the closest point from the
  // border to the attachment point. We look at the four vertices
  // and four edge centers.
  double d2, minD2, pt[3], minPt[3];
  minD2 = VTK_DOUBLE_MAX;

  pt[0] = p2[0]; pt[1] = p2[1]; pt[2] = minPt[2] = 0.0;
  if ( (d2 = vtkMath::Distance2BetweenPoints(p1,pt)) < minD2 )
    {
    minD2 = d2;
    minPt[0] = pt[0]; minPt[1] = pt[1]; 
    }

  pt[0] = (p2[0]+p3[0])/2.0;
  if ( (d2 = vtkMath::Distance2BetweenPoints(p1,pt)) < minD2 )
    {
    minD2 = d2;
    minPt[0] = pt[0]; minPt[1] = pt[1]; 
    }

  pt[0] = p3[0];
  if ( (d2 = vtkMath::Distance2BetweenPoints(p1,pt)) < minD2 )
    {
    minD2 = d2;
    minPt[0] = pt[0]; minPt[1] = pt[1]; 
    }

  pt[1] = (p2[1]+p3[1])/2.0;
  if ( (d2 = vtkMath::Distance2BetweenPoints(p1,pt)) < minD2 )
    {
    minD2 = d2;
    minPt[0] = pt[0]; minPt[1] = pt[1]; 
    }

  pt[1] = p3[1];
  if ( (d2 = vtkMath::Distance2BetweenPoints(p1,pt)) < minD2 )
    {
    minD2 = d2;
    minPt[0] = pt[0]; minPt[1] = pt[1]; 
    }

  pt[0] = (p2[0]+p3[0])/2.0;
  if ( (d2 = vtkMath::Distance2BetweenPoints(p1,pt)) < minD2 )
    {
    minD2 = d2;
    minPt[0] = pt[0]; minPt[1] = pt[1]; 
    }

  pt[0] = p2[0];
  if ( (d2 = vtkMath::Distance2BetweenPoints(p1,pt)) < minD2 )
    {
    minD2 = d2;
    minPt[0] = pt[0]; minPt[1] = pt[1]; 
    }

  pt[1] = (p2[1]+p3[1])/2.0;
  if ( (d2 = vtkMath::Distance2BetweenPoints(p1,pt)) < minD2 )
    {
    minD2 = d2;
    minPt[0] = pt[0]; minPt[1] = pt[1]; 
    }

  // Set the leader coordinates in appropriate coordinate system
  // The pipeline is connected differently depending on the dimension
  // and availability of a leader head.
  if ( this->Leader )
    {
    pts = this->LeaderPolyData->GetPoints();

    w1 = this->AttachmentPointCoordinate->GetComputedWorldValue(viewport);
    viewport->SetWorldPoint(w1[0],w1[1],w1[2],1.0);
    viewport->WorldToView();
    viewport->GetViewPoint(p1);
    
    viewport->SetDisplayPoint(minPt[0],minPt[1],0.0);
    viewport->DisplayToView();
    viewport->GetViewPoint(p2);
    p2[2] = p1[2];
    viewport->SetViewPoint(p2);
    viewport->ViewToWorld();
    double w3[4];
    viewport->GetWorldPoint(w3);
    if ( w3[3] != 0.0 )
      {
      w3[0] /= w3[3]; w3[1] /= w3[3]; w3[2] /= w3[3];
      }
    w2 = w3;
    
    pts->SetPoint(0, w1);
    pts->SetPoint(1, w2);
    this->HeadPolyData->GetPoints()->SetPoint(0,w1);
    this->HeadPolyData->GetPointData()->
      GetVectors()->SetTuple3(0,w1[0]-w2[0],w1[1]-w2[1],w1[2]-w2[2]);

    pts->Modified();
    this->HeadPolyData->Modified();
    }

  if ( this->LeaderGlyph )
    {
    // compute the scale
    this->LeaderGlyph->Update();
    double length = this->LeaderGlyph->GetLength();
    int *sze = viewport->GetSize();
    int   numPixels = static_cast<int> (this->LeaderGlyphSize * 
      sqrt(static_cast<double>(sze[0]*sze[0] + sze[1]*sze[1])));
    numPixels = (numPixels > this->MaximumLeaderGlyphSize ?
                 this->MaximumLeaderGlyphSize : numPixels );

    // determine the number of units length per pixel
    viewport->SetDisplayPoint(sze[0]/2,sze[1]/2,0);
    viewport->DisplayToWorld();
    viewport->GetWorldPoint(p1);
    if ( p1[3] != 0.0 ) {p1[0] /= p1[3]; p1[1] /= p1[3]; p1[2] /= p1[3];}

    viewport->SetDisplayPoint(sze[0]/2+1,sze[1]/2+1,0);
    viewport->DisplayToWorld();
    viewport->GetWorldPoint(p2);
    if ( p2[3] != 0.0 ) {p2[0] /= p2[3]; p2[1] /= p2[3]; p2[2] /= p2[3];}

    // Arbitrary 1.5 factor makes up for the use of "diagonals" in length
    // calculations; otherwise the scale factor tends to be too small
    double sf = 1.5 * numPixels * 
      sqrt(vtkMath::Distance2BetweenPoints(p1,p2)) / length;

    vtkDebugMacro(<<"Scale factor: " << sf);

    this->HeadGlyph->SetSource(this->LeaderGlyph);
    this->HeadGlyph->SetScaleFactor(sf);

    this->LeaderMapper2D->SetInput(this->AppendLeader->GetOutput());
    this->LeaderMapper3D->SetInput(this->AppendLeader->GetOutput());
    this->AppendLeader->Update();
    }
  else
    {
    this->LeaderMapper2D->SetInput(this->LeaderPolyData);
    this->LeaderMapper3D->SetInput(this->LeaderPolyData);
    this->LeaderPolyData->Update();
    }

  // assign properties
  //
  this->CaptionActor->SetInput(this->Caption);

  this->CaptionActor->SetProperty(this->GetProperty());
  this->BorderActor->SetProperty(this->GetProperty());
  this->BorderFillActor->SetProperty(this->GetProperty());
  this->LeaderActor2D->SetProperty(this->GetProperty());
  this->LeaderActor3D->GetProperty()->SetColor(
    this->GetProperty()->GetColor());

  // Okay we are ready to render something
  int renderedSomething = 0;
  if ( this->Border )
    {
	this->BorderFillActor->GetProperty()->SetColor(0.5,0.5,0.5);
	// dla
	renderedSomething += this->BorderFillActor->RenderOpaqueGeometry(viewport);
	this->BorderActor->GetProperty()->SetColor(1,1,1);
	// dla
	renderedSomething += this->BorderActor->RenderOpaqueGeometry(viewport);
    }
  if ( this->Leader )
    {
    if ( this->ThreeDimensionalLeader )
      {
      renderedSomething += this->LeaderActor3D->RenderOpaqueGeometry(viewport);
      }
    else
      {
      renderedSomething += this->LeaderActor2D->RenderOpaqueGeometry(viewport);
      }
    }
	renderedSomething += this->CaptionActor->RenderOpaqueGeometry(viewport);

  return renderedSomething;
}

//----------------------------------------------------------------------------
void vtkFilledCaptionActor2D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  if (this->CaptionTextProperty)
    {
    os << indent << "Caption Text Property:\n";
    this->CaptionTextProperty->PrintSelf(os,indent.GetNextIndent());
    }
  else
    {
    os << indent << "Caption Text Property: (none)\n";
    }

  os << indent << "Caption: ";
  if ( this->Caption )
    {
    os << this->Caption << "\n";
    }
  else
    {
    os << "(none)\n";
    }

  os << indent << "Leader: " << (this->Leader ? "On\n" : "Off\n");
  os << indent << "Three Dimensional Leader: " 
     << (this->ThreeDimensionalLeader ? "On\n" : "Off\n");
  os << indent << "Leader Glyph Size: " 
     << this->LeaderGlyphSize << "\n";
  os << indent << "MaximumLeader Glyph Size: " 
     << this->MaximumLeaderGlyphSize << "\n";
  if ( ! this->LeaderGlyph )
    {
    os << indent << "Leader Glyph: (none)\n";
    }
  else
    {
    os << indent << "Leader Glyph: (" << this->LeaderGlyph << ")\n";
    }
  os << indent << "Padding: " << this->Padding << "\n";
  os << indent << "Border: " << (this->Border ? "On\n" : "Off\n");
}

//----------------------------------------------------------------------------
void vtkFilledCaptionActor2D::ShallowCopy(vtkProp *prop)
{
  vtkFilledCaptionActor2D *a = vtkFilledCaptionActor2D::SafeDownCast(prop);
  if ( a != NULL )
    {
    this->SetCaption(a->GetCaption());
    this->SetAttachmentPoint(a->GetAttachmentPoint());
    this->SetBorder(a->GetBorder());
    this->SetLeader(a->GetLeader());
    this->SetThreeDimensionalLeader(a->GetThreeDimensionalLeader());
    this->SetLeaderGlyph(a->GetLeaderGlyph());
    this->SetLeaderGlyphSize(a->GetLeaderGlyphSize());
    this->SetMaximumLeaderGlyphSize(a->GetMaximumLeaderGlyphSize());
    this->SetPadding(a->GetPadding());
    this->SetCaptionTextProperty(a->GetCaptionTextProperty());
    }

  // Now do superclass
  this->vtkActor2D::ShallowCopy(prop);
}
