// Copyright 2019 Neil Fang. All Rights Reserved.

#include "MaterialExpressionTextureSampleParameterAnim.h"
#include "AnimatedTexture2D.h"

#include "UObject/ConstructorHelpers.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2DDynamic.h"
#include "Engine/TextureCube.h"
#include "Engine/TextureRenderTargetCube.h"

#define LOCTEXT_NAMESPACE "MaterialExpression"


UMaterialExpressionTextureSampleParameterAnim::UMaterialExpressionTextureSampleParameterAnim(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinder<UTexture2D> DefaultTexture;
		FText NAME_Texture;
		FText NAME_Parameters;
		FConstructorStatics()
			: DefaultTexture(TEXT("/Engine/EngineResources/DefaultTexture"))
			, NAME_Texture(LOCTEXT("Texture", "Texture"))
			, NAME_Parameters(LOCTEXT("Parameters", "Parameters"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	Texture = ConstructorStatics.DefaultTexture.Object;

#if WITH_EDITORONLY_DATA
	MenuCategories.Empty();
	MenuCategories.Add(ConstructorStatics.NAME_Texture);
	MenuCategories.Add(ConstructorStatics.NAME_Parameters);
#endif
}

#if WITH_EDITOR
void UMaterialExpressionTextureSampleParameterAnim::GetCaption(TArray<FString>& OutCaptions) const
{
	OutCaptions.Add(TEXT("ParamAnimTexture"));
	OutCaptions.Add(FString::Printf(TEXT("'%s'"), *ParameterName.ToString()));
}


#if ENGINE_MAJOR_VERSION <= 4 && ENGINE_MINOR_VERSION <= 22
bool UMaterialExpressionTextureSampleParameterAnim::TextureIsValid(UTexture* InTexture)
{
	bool Result = false;
	if (InTexture)
	{
		if (InTexture->IsA(UAnimatedTexture2D::StaticClass()))
		{
			Result = true;
		}

		if (InTexture->IsA(UTexture2D::StaticClass()))
		{
			Result = true;
		}
		if (InTexture->IsA(UTextureRenderTarget2D::StaticClass()))
		{
			Result = true;
		}
		if (InTexture->IsA(UTexture2DDynamic::StaticClass()))
		{
			Result = true;
		}
		if (InTexture->GetMaterialType() == MCT_TextureExternal)
		{
			Result = true;
		}
	}
	return Result;
}

const TCHAR* UMaterialExpressionTextureSampleParameterAnim::GetRequirements()
{
	return TEXT("Requires AnimatedTexture2D");
}
#else
bool UMaterialExpressionTextureSampleParameterAnim::TextureIsValid(UTexture* InTexture, FString& OutMessage)
{
	bool Result = false;
	if (InTexture)
	{
		if (InTexture->IsA(UAnimatedTexture2D::StaticClass()))
		{
			Result = true;
		}

		if (InTexture->IsA(UTexture2D::StaticClass()))
		{
			Result = true;
		}
		if (InTexture->IsA(UTextureRenderTarget2D::StaticClass()))
		{
			Result = true;
		}
		if (InTexture->IsA(UTexture2DDynamic::StaticClass()))
		{
			Result = true;
		}
		if (InTexture->GetMaterialType() == MCT_TextureExternal)
		{
			Result = true;
		}

		if (!Result)
			OutMessage = TEXT("Invalid texture type");
	}
	else
	{
		OutMessage = TEXT("NULL Textue");
	}

	return Result;
}
#endif

void UMaterialExpressionTextureSampleParameterAnim::SetDefaultTexture()
{
	Texture = LoadObject<UTexture2D>(NULL, TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture"), NULL, LOAD_None, NULL);
}
#endif // WITH_EDITOR
#undef LOCTEXT_NAMESPACE
