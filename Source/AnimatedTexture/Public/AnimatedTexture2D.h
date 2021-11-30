/**
 * Copyright 2019 Neil Fang. All Rights Reserved.
 *
 * Animated Texture from GIF file
 *
 * Created by Neil Fang
 * GitHub Repo: https://github.com/neil3d/UnrealAnimatedTexturePlugin
 * GitHub Page: http://neil3d.github.io
 *
*/

#pragma once

#include "CoreMinimal.h"
#include "Tickable.h"	// Engine
#include "Engine/Texture.h"	// Engine

#include "AnimatedTexture2D.generated.h"

class FAnimatedTextureResource;
ANIMATEDTEXTURE_API bool isGifData(const void* data);

USTRUCT()
struct FGIFFrame
{
	GENERATED_BODY()
public:
	UPROPERTY()
		float Time;	// next frame delay in sec
	UPROPERTY()
		uint32 Index;	// 0-based index of the current frame
	UPROPERTY()
		uint32 Width;	// current frame width
	UPROPERTY()
		uint32 Height;	// current frame height
	UPROPERTY()
		uint32 OffsetX;	// current frame horizontal offset
	UPROPERTY()
		uint32 OffsetY;	// current frame vertical offset
	UPROPERTY()
		bool Interlacing;	// see: https://en.wikipedia.org/wiki/GIF#Interlacing
	UPROPERTY()
		uint8 Mode;	// next frame (sic next, not current) blending mode
	UPROPERTY()
		int16 TransparentIndex;	// 0-based transparent color index (or −1 when transparency is disabled)
	UPROPERTY()
		TArray<uint8> PixelIndices;	// pixel indices for the current frame
	UPROPERTY()
		TArray<FColor> Palette;	// the current palette

	FGIFFrame() :Time(0), Index(0), Width(0), Height(0), OffsetX(0), OffsetY(0),
		Interlacing(false), Mode(0), TransparentIndex(-1)
	{}
};


/**
 *
 */
UCLASS(BlueprintType, Category = AnimatedTexture, hideCategories = (Adjustments, Compression, LevelOfDetail))
class ANIMATEDTEXTURE_API UAnimatedTexture2D : public UTexture
{
	GENERATED_BODY()

public:
	friend FAnimatedTextureResource;

	UAnimatedTexture2D(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimatedTexture, meta = (DisplayName = "X-axis Tiling Method"), AssetRegistrySearchable, AdvancedDisplay)
		TEnumAsByte<enum TextureAddress> AddressX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimatedTexture, meta = (DisplayName = "Y-axis Tiling Method"), AssetRegistrySearchable, AdvancedDisplay)
		TEnumAsByte<enum TextureAddress> AddressY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimatedTexture)
		bool SupportsTransparency = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimatedTexture)
		float DefaultFrameDelay = 1.0f / 10;	// used while Frame.Delay==0

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimatedTexture)
		float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimatedTexture)
		bool bLooping = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimatedTexture)
		bool bAlwaysTickEvenNoSee = false;

	UPROPERTY(VisibleAnywhere, Transient,Category = AnimatedTexture)
		int FrameNum;


	virtual void PostLoad() override;


	virtual void BeginDestroy() override;


	bool ImportGIF(const uint8* Buffer, uint32 BufferSize);

	void ResetToInVaildGif()
	{
		GlobalWidth = 0;
		GlobalHeight = 0;
		Background = 0;
		Duration = 0.0f;
		Frames.Empty();
		FrameNum = 0;
	}

	void Import_Init(uint32 InGlobalWidth, uint32 InGlobalHeight, uint8 InBackground, uint32 InFrameCount);

	int GetFrameCount() const
	{ 
		return Frames.Num(); 
	}
	
	FGIFFrame& GetFrame(int32 Index) {
		return Frames[Index];
	}

	float GetFrameDelay(int FrameIndex) const
	{
		const FGIFFrame& Frame = Frames[FrameIndex];
		return Frame.Time;
	}

	float GetTotalDuration() const { return Duration; }


	void Import_Finished();

	void PostInitProperties() override;

private:
	bool ParseRawData();

public:
	UFUNCTION(BlueprintCallable, Category = AnimatedTexture)
		void Play();

	UFUNCTION(BlueprintCallable, Category = AnimatedTexture)
		void PlayFromStart();

	UFUNCTION(BlueprintCallable, Category = AnimatedTexture)
		void Stop();

	UFUNCTION(BlueprintCallable, Category = AnimatedTexture)
		bool IsPlaying() const { return bPlaying; }

	UFUNCTION(BlueprintCallable, Category = AnimatedTexture)
		void SetLooping(bool bNewLooping) { bLooping = bNewLooping; }

	UFUNCTION(BlueprintCallable, Category = AnimatedTexture)
		bool IsLooping() const { return bLooping; }

	UFUNCTION(BlueprintCallable, Category = AnimatedTexture)
		void SetPlayRate(float NewRate) { PlayRate = NewRate; }

	UFUNCTION(BlueprintCallable, Category = AnimatedTexture)
		float GetPlayRate() const { return PlayRate; }

	UFUNCTION(BlueprintCallable, Category = AnimatedTexture)
		float GetAnimationLength() const;


	//~ Begin UTexture Interface.
	virtual float GetSurfaceWidth() const override;
	virtual float GetSurfaceHeight() const override;
	virtual FTextureResource* CreateResource() override;
	virtual EMaterialValueType GetMaterialType() const override { return MCT_Texture2D; }
	virtual uint32 CalcTextureMemorySizeEnum(ETextureMipCount Enum) const override;


	//~ End UTexture Interface.


	//~ Begin UObject Interface.
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
	virtual void GetResourceSizeEx(FResourceSizeEx& CumulativeResourceSize) override;
	
	//~ End UObject Interface.

protected:
	UPROPERTY()
		bool bPlaying = true;
private:
	//UPROPERTY()
		uint32 GlobalWidth = 0;

	//UPROPERTY()
		uint32 GlobalHeight = 0;

	//UPROPERTY()
		uint8 Background = 0;	// 0-based background color index for the current palette

	//UPROPERTY()
		float Duration = 0.0f;
	//UPROPERTY()
	TArray<FGIFFrame> Frames;

	UPROPERTY()
	TArray<uint8> RawData;
};