// Copyright 2019 Neil Fang. All Rights Reserved.

#include "AnimatedTexture2D.h"
#include "AnimatedTextureResource.h"
#include "gif_load/gif_load.h" // from: https://github.com/hidefromkgb/gif_load

bool isGifData(const void* data) {
	return FMemory::Memcmp(data, "GIF", 3) == 0;
}


//ANIMATEDTEXTURE_API bool LoadGIFBinary(UAnimatedTexture2D* OutGIF, const uint8* Buffer, uint32 BufferSize);

void GIFFrameLoader1(void* data, struct GIF_WHDR* whdr)
{
	UAnimatedTexture2D* OutGIF = (UAnimatedTexture2D*)data;

	//-- init on first frame
	if (OutGIF->GetFrameCount() == 0) {
		OutGIF->Import_Init(whdr->xdim, whdr->ydim, whdr->bkgd, whdr->nfrm);
	}

	//-- import frame
	int FrameIndex = whdr->ifrm;

	check(OutGIF->GetFrameCount() == whdr->nfrm);
	check(FrameIndex >= 0 && FrameIndex < OutGIF->GetFrameCount());

	FGIFFrame& Frame = OutGIF->GetFrame(FrameIndex);

	//-- copy properties
	if (whdr->time >= 0)
		Frame.Time = whdr->time * 0.01f;	// 1 GIF time units = 10 msec
	else
		Frame.Time = (-whdr->time - 1) * 0.01f;

	/** [TODO:] the frame is assumed to be inside global bounds,
			however it might exceed them in some GIFs; fix me. **/
	Frame.Index = whdr->ifrm;
	Frame.Width = whdr->frxd;
	Frame.Height = whdr->fryd;
	Frame.OffsetX = whdr->frxo;
	Frame.OffsetY = whdr->fryo;
	Frame.Interlacing = whdr->intr != 0;
	Frame.Mode = whdr->mode;
	Frame.TransparentIndex = whdr->tran;

	//-- copy pixel data
	int NumPixel = Frame.Width * Frame.Height;
	Frame.PixelIndices.SetNumUninitialized(NumPixel);
	FMemory::Memcpy(Frame.PixelIndices.GetData(), whdr->bptr, NumPixel);

	//-- copy pal
	int PaletteSize = whdr->clrs;
	Frame.Palette.Init(FColor(0, 0, 0, 255), PaletteSize);
	for (int i = 0; i < PaletteSize; i++)
	{
		FColor& uc = Frame.Palette[i];
		uc.R = whdr->cpal[i].R;
		uc.G = whdr->cpal[i].G;
		uc.B = whdr->cpal[i].B;
	}// end of for
}


bool LoadGIFBinary(UAnimatedTexture2D* OutGIF, const uint8* Buffer, uint32 BufferSize)
{
	int Ret = GIF_Load((void*)Buffer, BufferSize, GIFFrameLoader1, 0, (void*)OutGIF, 0L);
	OutGIF->Import_Finished();

	if (Ret < 0) {
		UE_LOG(LogTexture, Warning, TEXT("gif format error."));
		return false;
	}
	return true;
}

float UAnimatedTexture2D::GetSurfaceWidth() const
{
	return GlobalWidth;
}

float UAnimatedTexture2D::GetSurfaceHeight() const
{
	return GlobalHeight;
}

FTextureResource* UAnimatedTexture2D::CreateResource()
{
	FTextureResource* NewResource = new FAnimatedTextureResource(this);
	return NewResource;
}

uint32 UAnimatedTexture2D::CalcTextureMemorySizeEnum(ETextureMipCount Enum) const
{
	if(GlobalWidth>0 && GlobalHeight>0) 
	{

		uint32 Flags = SRGB ? TexCreate_SRGB : 0;
		uint32 NumMips = 1;
		uint32 NumSamples = 1;
		uint32 TextureAlign;
		FRHIResourceCreateInfo CreateInfo;
		uint32 Size = (uint32)RHICalcTexture2DPlatformSize(GlobalWidth, GlobalHeight, PF_B8G8R8A8, 1, 1, (ETextureCreateFlags)Flags, FRHIResourceCreateInfo(), TextureAlign);

		return Size;
	}

	return 4;
}

#if WITH_EDITOR
void UAnimatedTexture2D::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	bool RequiresNotifyMaterials = false;
	bool ResetAnimState = false;

	UProperty* PropertyThatChanged = PropertyChangedEvent.Property;
	if (PropertyThatChanged)
	{
		const FName PropertyName = PropertyThatChanged->GetFName();

		static const FName SupportsTransparencyName = GET_MEMBER_NAME_CHECKED(UAnimatedTexture2D, SupportsTransparency);

		if (PropertyName == SupportsTransparencyName)
		{
			RequiresNotifyMaterials = true;
			ResetAnimState = true;
		}
	}// end of if(prop is valid)

	if (ResetAnimState)
	{
		//AnimState = FAnmatedTextureState();
		//AnimSource->DecodeFrameToRHI(Resource, AnimState, SupportsTransparency);
	}

	if (RequiresNotifyMaterials)
		NotifyMaterials();
}
#endif // WITH_EDITOR

void UAnimatedTexture2D::GetResourceSizeEx(FResourceSizeEx& CumulativeResourceSize)
{
	Super::GetResourceSizeEx(CumulativeResourceSize);

	//if (CumulativeResourceSize.GetResourceSizeMode() == EResourceSizeMode::Exclusive)
	{
		CumulativeResourceSize.AddDedicatedSystemMemoryBytes(Frames.GetAllocatedSize());

		for (auto& Frame: Frames)
		{
			CumulativeResourceSize.AddDedicatedSystemMemoryBytes(Frame.Palette.GetAllocatedSize());
			CumulativeResourceSize.AddDedicatedSystemMemoryBytes(Frame.PixelIndices.GetAllocatedSize());
		}
	}
}



float UAnimatedTexture2D::GetAnimationLength() const
{
	return Duration;
}


UAnimatedTexture2D::UAnimatedTexture2D(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
:Super(ObjectInitializer)
{

}

void UAnimatedTexture2D::PostLoad()
{
	ParseRawData();
	Super::PostLoad();
}

void UAnimatedTexture2D::BeginDestroy() 
{
	Super::BeginDestroy();
}


bool UAnimatedTexture2D::ImportGIF(const uint8* Buffer, uint32 BufferSize)
{
	Frames.Empty();
	RawData.SetNumUninitialized(BufferSize);
	FMemory::Memcpy(RawData.GetData(), Buffer, BufferSize);

	return ParseRawData();
}

void UAnimatedTexture2D::Import_Init(uint32 InGlobalWidth, uint32 InGlobalHeight, uint8 InBackground, uint32 InFrameCount)
{
	GlobalWidth = InGlobalWidth;
	GlobalHeight = InGlobalHeight;
	Background = InBackground;


	Frames.SetNum(InFrameCount);
	FrameNum = InFrameCount;
}

void UAnimatedTexture2D::Import_Finished()
{
	Duration = 0.0f;
	for (const auto& Frm : Frames)
		Duration += Frm.Time;
}

void UAnimatedTexture2D::PostInitProperties()
{
	Super::PostInitProperties();
}

bool UAnimatedTexture2D::ParseRawData()
{
	int Ret = GIF_Load((void*)RawData.GetData(), RawData.Num(), GIFFrameLoader1, 0, (void*)this, 0L);
	this->Import_Finished();

	if (Ret < 0) {
		UE_LOG(LogTexture, Warning, TEXT("gif format error."));
		return false;
	}
	return true;
}

void UAnimatedTexture2D::Play()
{
	bPlaying = true;
}

void UAnimatedTexture2D::PlayFromStart()
{
	bPlaying = true;
}

void UAnimatedTexture2D::Stop()
{
	bPlaying = false;
}
