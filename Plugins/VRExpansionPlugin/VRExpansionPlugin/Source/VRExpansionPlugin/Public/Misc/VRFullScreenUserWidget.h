// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/WidgetComponent.h"
//#include "CompositingElement.h"
#include "VRFullScreenUserWidget.generated.h"

class FWidgetRenderer;
class FVRWidgetPostProcessHitTester;
class SConstraintCanvas;
class SVirtualWindow;
class SViewport;
class SWidget;
class ULevel;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UPostProcessComponent;
class UTextureRenderTarget2D;
class UWorld;

#if WITH_EDITOR
class SLevelViewport;
#endif


UENUM(BlueprintType)
enum class EVRWidgetDisplayType : uint8
{
	/** Do not display. */
	Inactive,
	/** Display on a game viewport. */
	Viewport,
	/** Display as a post process. */
	PostProcess,
	/** Render to a texture and send to composure. */
   // Composure,
};


USTRUCT()
struct FVRFullScreenUserWidget_Viewport
{
	GENERATED_BODY()

public:
	FVRFullScreenUserWidget_Viewport();
	bool Display(UWorld* World, UUserWidget* Widget, float InDPIScale);
	void Hide(UWorld* World);
	void Tick(UWorld* World, float DeltaSeconds);

#if WITH_EDITOR
	/** If set, use this viewport instead of GetFirstActiveLevelViewport() */
	TWeakPtr<SLevelViewport> TargetViewport;
#endif

private:
	bool bAddedToGameViewport;

	/** Constraint widget that contains the widget we want to display. */
	TWeakPtr<SConstraintCanvas> FullScreenCanvasWidget;

#if WITH_EDITOR
	/** Level viewport the widget was added to. */
	TWeakPtr<SLevelViewport> OverlayWidgetLevelViewport;
#endif
};

USTRUCT()
struct FVRFullScreenUserWidget_PostProcess
{
	GENERATED_BODY()

public:
	FVRFullScreenUserWidget_PostProcess();
	bool Display(UWorld* World, UUserWidget* Widget, bool bInRenderToTextureOnly, float InDPIScale);
	void Hide(UWorld* World);
	void Tick(UWorld* World, float DeltaSeconds);

	TSharedPtr<SVirtualWindow> VREXPANSIONPLUGIN_API GetSlateWindow() const;

private:
	bool CreatePostProcessComponent(UWorld* World);
	void ReleasePostProcessComponent();

	bool CreateRenderer(UWorld* World, UUserWidget* Widget, float InDPIScale);
	void ReleaseRenderer();
	void TickRenderer(UWorld* World, float DeltaSeconds);

	FIntPoint CalculateWidgetDrawSize(UWorld* World);
	bool IsTextureSizeValid(FIntPoint Size) const;

	void RegisterHitTesterWithViewport(UWorld* World);
	void UnRegisterHitTesterWithViewport();

public:
	/**
	 * Post process material used to display the widget.
	 * SlateUI [Texture]
	 * TintColorAndOpacity [Vector]
	 * OpacityFromTexture [Scalar]
	 */
	UPROPERTY(EditAnywhere, Category = PostProcess)
	UMaterialInterface* PostProcessMaterial;

	/** Tint color and opacity for this component. */
	UPROPERTY(EditAnywhere, Category = PostProcess)
	FLinearColor PostProcessTintColorAndOpacity;

	/** Sets the amount of opacity from the widget's UI texture to use when rendering the translucent or masked UI to the viewport (0.0-1.0). */
	UPROPERTY(EditAnywhere, Category = PostProcess, meta = (ClampMin = 0.0f, ClampMax = 1.0f))
	float PostProcessOpacityFromTexture;

	/** The size of the rendered widget. */
	UPROPERTY(EditAnywhere, Category = PostProcess, meta=(InlineEditConditionToggle))
	bool bWidgetDrawSize;

	/** The size of the rendered widget. */
	UPROPERTY(EditAnywhere, Category = PostProcess, meta=(EditCondition= bWidgetDrawSize))
	FIntPoint WidgetDrawSize;

	/** Is the virtual window created to host the widget focusable? */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = PostProcess)
	bool bWindowFocusable;

	/** The visibility of the virtual window created to host the widget. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = PostProcess)
	EWindowVisibility WindowVisibility;

	/** Register with the viewport for hardware input from the mouse and keyboard. It can and will steal focus from the viewport. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category= PostProcess)
	bool bReceiveHardwareInput;

	/** The background color of the render target */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = PostProcess)
	FLinearColor RenderTargetBackgroundColor;

	/** The blend mode for the widget. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = PostProcess)
	EWidgetBlendMode RenderTargetBlendMode;

	/** List of composure layers that are expecting to use the WidgetRenderTarget. */
	//UPROPERTY(EditAnywhere, Category= PostProcess)
    //TArray<ACompositingElement*> ComposureLayerTargets;

	/** The target to which the user widget is rendered. */
	UPROPERTY(Transient)
    UTextureRenderTarget2D* WidgetRenderTarget;

	/** Only render to the UTextureRenderTarget2D - do not output to the final viewport. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = PostProcess)
		bool bRenderToTextureOnly;

#if WITH_EDITOR
	/** If set, use this viewport instead of GetFirstActiveLevelViewport() */
	TWeakPtr<SLevelViewport> TargetViewport;
#endif
private:
	/** Post process component used to add the material to the post process chain. */
	UPROPERTY(Transient)
	UPostProcessComponent* PostProcessComponent;

	/** The dynamic instance of the material that the render target is attached to. */
	UPROPERTY(Transient)
	UMaterialInstanceDynamic* PostProcessMaterialInstance;

	/** The slate window that contains the user widget content. */
	TSharedPtr<SVirtualWindow> SlateWindow;

	/** The slate viewport we are registered to. */
	TWeakPtr<SViewport> ViewportWidget;

	/** Helper class for drawing widgets to a render target. */
	FWidgetRenderer* WidgetRenderer;

	/** The size of the rendered widget */
	FIntPoint CurrentWidgetDrawSize;

	/** Hit tester when we want the hardware input. */
	TSharedPtr<FVRWidgetPostProcessHitTester> CustomHitTestPath;

};

/**
 * Will set the Widgets on a viewport either by Widgets are first rendered to a render target, then that render target is displayed in the world.
 */
UCLASS(meta=(ShowOnlyInnerProperties))
class VREXPANSIONPLUGIN_API UVRFullScreenUserWidget : public UObject
{
	GENERATED_BODY()

public:
	UVRFullScreenUserWidget(const FObjectInitializer& ObjectInitializer);

	//~ Begin UObject interface
	virtual void BeginDestroy() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End UObject Interface

	bool ShouldDisplay(UWorld* World) const;
	EVRWidgetDisplayType GetDisplayType(UWorld* World) const;
	bool IsDisplayed() const;

	virtual bool Display(UWorld* World);
	virtual void Hide();
	virtual void Tick(float DeltaTime);

	void SetDisplayTypes(EVRWidgetDisplayType InEditorDisplayType, EVRWidgetDisplayType InGameDisplayType, EVRWidgetDisplayType InPIEDisplayType);

protected:
	void InitWidget();
	void ReleaseWidget();

	FVector2D FindSceneViewportSize();
	float GetViewportDPIScale();

private:
	void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld);

protected:
	/** The display type when the world is an editor world. */
	UPROPERTY(EditAnywhere, Category = "User Interface")
	EVRWidgetDisplayType EditorDisplayType;

	/** The display type when the world is a game world. */
	UPROPERTY(EditAnywhere, Category = "User Interface")
	EVRWidgetDisplayType GameDisplayType;

	/** The display type when the world is a PIE world. */
	UPROPERTY(EditAnywhere, Category = "User Interface", meta = (DisplayName = "PIE Display Type"))
	EVRWidgetDisplayType PIEDisplayType;

	/** Behavior when the widget should be display by the slate attached to the viewport. */
	UPROPERTY(EditAnywhere, Category = "Viewport", meta = (ShowOnlyInnerProperties))
	FVRFullScreenUserWidget_Viewport ViewportDisplayType;

public:
	/** The class of User Widget to create and display an instance of */
	UPROPERTY(EditAnywhere, Category = "User Interface")
	TSubclassOf<UUserWidget> WidgetClass;

	/** Behavior when the widget should be display by a post process. */
	UPROPERTY(EditAnywhere, Category = "Post Process", meta = (ShowOnlyInnerProperties))
	FVRFullScreenUserWidget_PostProcess PostProcessDisplayType;

	// Get a pointer to the inner widget.
	// Note: This should not be stored!
	UUserWidget* GetWidget() const { return Widget; };

#if WITH_EDITOR
	/** If set, use this viewport instead of GetFirstActiveLevelViewport() */
	TWeakPtr<SLevelViewport> TargetViewport;

	/** Sets the TargetViewport to use on both the Viewport and the PostProcess class */
	void SetAllTargetViewports(TWeakPtr<SLevelViewport> InTargetViewport);

	/** Resets the TargetViewport  */
	void ResetAllTargetViewports();
#endif

private:
	/** The User Widget object displayed and managed by this component */
	UPROPERTY(Transient, DuplicateTransient)
	UUserWidget* Widget;

	/** The world the widget is attached to. */
	TWeakObjectPtr<UWorld> World;

	/** How we currently displaying the widget. */
	EVRWidgetDisplayType CurrentDisplayType;

	/** The user requested the widget to be displayed. It's possible that some setting are invalid and the widget will not be displayed. */
	bool bDisplayRequested;
};
