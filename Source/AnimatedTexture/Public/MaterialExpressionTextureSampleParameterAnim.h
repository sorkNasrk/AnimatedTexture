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
#include "UObject/ObjectMacros.h"
#include "Materials/MaterialExpressionTextureSampleParameter.h"
#include "Runtime/Launch/Resources/Version.h"

#include "MaterialExpressionTextureSampleParameterAnim.generated.h"

class UTexture;

UCLASS(collapsecategories, hidecategories = Object)
class ANIMATEDTEXTURE_API UMaterialExpressionTextureSampleParameterAnim : public UMaterialExpressionTextureSampleParameter
{
	GENERATED_UCLASS_BODY()

#if WITH_EDITOR
		//~ Begin UMaterialExpression Interface

		virtual void GetCaption(TArray<FString>& OutCaptions) const override;

	//~ End UMaterialExpression Interface

	//~ Begin UMaterialExpressionTextureSampleParameter Interface
#if ENGINE_MAJOR_VERSION <= 4 && ENGINE_MINOR_VERSION <= 22
	virtual bool TextureIsValid(UTexture* InTexture) override;
	virtual const TCHAR* GetRequirements() override;
#else
	virtual bool TextureIsValid(UTexture* InTexture, FString& OutMessage) override;
#endif
	virtual void SetDefaultTexture() override;
	//~ End UMaterialExpressionTextureSampleParameter Interface

#endif // WITH_EDITOR
};



