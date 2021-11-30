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
#include "TextureResource.h"	// Engine

class UAnimatedTexture2D;

struct FAnmatedTextureState {
	int CurrentFrame;
	float FrameTime;

	FAnmatedTextureState() :CurrentFrame(0), FrameTime(0) {}
};

/**
 * FTextureResource implementation for animated 2D textures
 */
class FAnimatedTextureResource : public FTextureResource, public FTickableObjectRenderThread
{
public:
	FAnimatedTextureResource(UAnimatedTexture2D* InOwner);

	//~ Begin FTextureResource Interface.
	virtual uint32 GetSizeX() const override;
	virtual uint32 GetSizeY() const override;
	virtual void InitRHI() override;
	virtual void ReleaseRHI() override;
	//~ End FTextureResource Interface.

	//~ Begin FTickableObjectRenderThread Interface.
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override
	{
		return true;
	}
	virtual TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UAnimatedTexture2D, STATGROUP_Tickables);
	}
	//~ End FTickableObjectRenderThread Interface.

	bool TickAnim(float DeltaTime);
	void DecodeFrameToRHI();


private:
	int32 GetDefaultMipMapBias() const;

	void CreateSamplerStates(float MipMapBias);

private:
	UAnimatedTexture2D* Owner;
	FAnmatedTextureState AnimState;
	TArray<FColor>	FrameBuffer[2];
	uint32 LastFrame;
};
