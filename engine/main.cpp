// C++ Standard Library
#include <cmath>
#include <ostream>

// SDE
#include "sde/app.hpp"
#include "sde/geometry_utils.hpp"
#include "sde/graphics/assets.hpp"
#include "sde/graphics/colors.hpp"
#include "sde/graphics/image.hpp"
#include "sde/graphics/render_target.hpp"
#include "sde/graphics/renderer.hpp"
#include "sde/graphics/shader.hpp"
#include "sde/graphics/shapes.hpp"
#include "sde/graphics/sprite.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/graphics/tile_map.hpp"
#include "sde/graphics/tile_set.hpp"
#include "sde/graphics/type_setter.hpp"
#include "sde/graphics/window.hpp"
#include "sde/logging.hpp"
#include "sde/view.hpp"

// clang-format off

static const auto* kSpriteShader = R"SpriteShader(

  layout (location = 0) in vec2 vPosition;
  layout (location = 1) in vec2 vTexCoord;
  layout (location = 2) in float vTexUnit;
  layout (location = 3) in vec4 vTintColor;

  out vec2 fTexCoord;
  out vec4 fTintColor;
  out float fTexUnit;

  uniform mat3 uCameraTransform;

  void main()
  {
    gl_Position = vec4(uCameraTransform * vec3(vPosition, 1), 1);
    fTexUnit = vTexUnit;
    fTexCoord = vTexCoord;
    fTintColor = vTintColor;
  }

  ---

  out vec4 FragColor;

  in float fTexUnit;
  in vec2 fTexCoord;
  in vec4 fTintColor;

  uniform sampler2D[16] uTexture;

  void main()
  {
    int texture_unit = int(fTexUnit);
    bool texture_enabled = bool(texture_unit >= 0);
    vec4 texture_color_sampled = texture2D(uTexture[texture_unit], fTexCoord);
    FragColor = (float(!texture_enabled) * fTintColor) + float(texture_enabled) * (fTintColor * texture_color_sampled);
  }
)SpriteShader";


static const auto* kTextShader = R"TextShader(

  layout (location = 0) in vec2 vPosition;
  layout (location = 1) in vec2 vTexCoord;
  layout (location = 2) in float vTexUnit;
  layout (location = 3) in vec4 vTintColor;

  out vec2 fTexCoord;
  out vec4 fTintColor;
  out float fTexUnit;

  uniform mat3 uCameraTransform;

  void main()
  {
    gl_Position = vec4(uCameraTransform * vec3(vPosition, 1), 1);
    fTexCoord = vTexCoord;
    fTintColor = vTintColor;
    fTexUnit = vTexUnit;
  }

  ---
  out vec4 FragColor;

  in float fTexUnit;
  in vec2 fTexCoord;
  in vec4 fTintColor;

  uniform sampler2D[16] uTexture;

  void main()
  {
    int u = int(fTexUnit);
    vec4 s = texture2D(uTexture[u], fTexCoord);
    FragColor = s[0] * fTintColor;
  }

)TextShader";



int main(int argc, char** argv)
{
  using namespace sde;
  using namespace sde::graphics;

  SDE_LOG_INFO("starting...");

  auto icon_or_error = Image::load("/home/brian/dev/assets/icons/red.png");
  SDE_ASSERT_TRUE(icon_or_error.has_value());

  auto app_or_error = App::create(
    {
      .initial_size = {1000, 500},
      .icon = std::addressof(*icon_or_error),
      //.cursor = std::addressof(*icon_or_error),
    });
  SDE_ASSERT_TRUE(app_or_error.has_value());

  Assets assets;

  auto player_font_or_error = assets.fonts.create("/home/brian/dev/assets/fonts/white_rabbit.ttf");
  SDE_ASSERT_TRUE(player_font_or_error.has_value());
  auto player_typeset_or_error = assets.type_sets.create(assets.textures, *player_font_or_error, TypeSetOptions{.height_px = 20});
  SDE_ASSERT_TRUE(player_typeset_or_error.has_value());

  auto text_shader_or_error = assets.shaders.create(kTextShader);
  SDE_ASSERT_TRUE(text_shader_or_error.has_value());

  TypeSetter type_setter{*player_typeset_or_error};
  RenderResources text_rendering_resources;
  text_rendering_resources.shader = (*text_shader_or_error);
  text_rendering_resources.buffer_group = 1;


  auto sprite_shader_or_error = assets.shaders.create(kSpriteShader);
  SDE_ASSERT_TRUE(sprite_shader_or_error.has_value());

  const auto loadTexture = [&assets](const auto& path)
  {
    // Load all texture source image
    auto texture_source_image = Image::load(path, {.flags = {.flip_vertically = true}});
    SDE_ASSERT_TRUE(texture_source_image.has_value());

    // Create atlas textures from image
    auto texture = assets.textures.create(*texture_source_image);
    SDE_ASSERT_TRUE(texture.has_value());

    return std::move(texture).value();
  };


  // Load all textures

  auto movement_front_atlas = loadTexture("/home/brian/dev/assets/sprites/red/Top Down/Front Movement.png");
  auto movement_back_atlas = loadTexture("/home/brian/dev/assets/sprites/red/Top Down/Back Movement.png");
  auto movement_side_atlas = loadTexture("/home/brian/dev/assets/sprites/red/Top Down/Side Movement.png");
  auto movement_back_side_atlas = loadTexture("/home/brian/dev/assets/sprites/red/Top Down/BackSide Movement.png");
  auto movement_front_side_atlas = loadTexture("/home/brian/dev/assets/sprites/red/Top Down/FrontSide Movement.png");


  // Create animation frame

  // IDLE ------------------------------------------

  auto idle_front_frames = assets.tile_sets.create(
    movement_front_atlas,
    TileSetSliceUniform{
      .tile_size_px = {64, 64},
      .tile_orientation_x = TileOrientation::kNormal,
      .tile_orientation_y = TileOrientation::kNormal,
      .direction = TileSliceDirection::kRowWise,
      .start_offset = 18,
      .stop_after = 6,
    });
  SDE_ASSERT_TRUE(idle_front_frames.has_value());

  auto idle_back_frames = assets.tile_sets.create(
    movement_back_atlas,
    TileSetSliceUniform{
      .tile_size_px = {64, 64},
      .tile_orientation_x = TileOrientation::kNormal,
      .tile_orientation_y = TileOrientation::kNormal,
      .direction = TileSliceDirection::kRowWise,
      .start_offset = 18,
      .stop_after = 6,
    });
  SDE_ASSERT_TRUE(idle_back_frames.has_value());

  auto idle_right_frames = assets.tile_sets.create(
    movement_side_atlas,
    TileSetSliceUniform{
      .tile_size_px = {64, 64},
      .tile_orientation_x = TileOrientation::kNormal,
      .tile_orientation_y = TileOrientation::kNormal,
      .direction = TileSliceDirection::kRowWise,
      .start_offset = 18,
      .stop_after = 6,
    });
  SDE_ASSERT_TRUE(idle_right_frames.has_value());

  auto idle_left_frames = assets.tile_sets.create(
    movement_side_atlas,
    TileSetSliceUniform{
      .tile_size_px = {64, 64},
      .tile_orientation_x = TileOrientation::kFlipped,
      .tile_orientation_y = TileOrientation::kNormal,
      .direction = TileSliceDirection::kRowWise,
      .start_offset = 18,
      .stop_after = 6,
    });
  SDE_ASSERT_TRUE(idle_left_frames.has_value());

  auto idle_front_right_frames = assets.tile_sets.create(
    movement_front_side_atlas,
    TileSetSliceUniform{
      .tile_size_px = {64, 64},
      .tile_orientation_x = TileOrientation::kNormal,
      .tile_orientation_y = TileOrientation::kNormal,
      .direction = TileSliceDirection::kRowWise,
      .start_offset = 12,
      .stop_after = 6,
    });
  SDE_ASSERT_TRUE(idle_front_right_frames.has_value());

  auto idle_front_left_frames = assets.tile_sets.create(
    movement_front_side_atlas,
    TileSetSliceUniform{
      .tile_size_px = {64, 64},
      .tile_orientation_x = TileOrientation::kFlipped,
      .tile_orientation_y = TileOrientation::kNormal,
      .direction = TileSliceDirection::kRowWise,
      .start_offset = 12,
      .stop_after = 6,
    });
  SDE_ASSERT_TRUE(idle_front_left_frames.has_value());

  auto idle_back_right_frames = assets.tile_sets.create(
    movement_back_side_atlas,
    TileSetSliceUniform{
      .tile_size_px = {64, 64},
      .tile_orientation_x = TileOrientation::kNormal,
      .tile_orientation_y = TileOrientation::kNormal,
      .direction = TileSliceDirection::kRowWise,
      .start_offset = 12,
      .stop_after = 6,
    });
  SDE_ASSERT_TRUE(idle_back_right_frames.has_value());

  auto idle_back_left_frames = assets.tile_sets.create(
    movement_back_side_atlas,
    TileSetSliceUniform{
      .tile_size_px = {64, 64},
      .tile_orientation_x = TileOrientation::kFlipped,
      .tile_orientation_y = TileOrientation::kNormal,
      .direction = TileSliceDirection::kRowWise,
      .start_offset = 12,
      .stop_after = 6,
    });
  SDE_ASSERT_TRUE(idle_back_left_frames.has_value());


  // RUNNING ----------------------------------------

  auto walking_front_frames = assets.tile_sets.create(
    movement_front_atlas,
    TileSetSliceUniform{
      .tile_size_px = {64, 64},
      .tile_orientation_x = TileOrientation::kNormal,
      .tile_orientation_y = TileOrientation::kNormal,
      .direction = TileSliceDirection::kRowWise,
      .start_offset = 12,
      .stop_after = 6,
    });
  SDE_ASSERT_TRUE(walking_front_frames.has_value());

  auto walking_back_frames = assets.tile_sets.create(
    movement_back_atlas,
    TileSetSliceUniform{
      .tile_size_px = {64, 64},
      .tile_orientation_x = TileOrientation::kNormal,
      .tile_orientation_y = TileOrientation::kNormal,
      .direction = TileSliceDirection::kRowWise,
      .start_offset = 12,
      .stop_after = 6,
    });
  SDE_ASSERT_TRUE(walking_back_frames.has_value());

  auto walking_right_frames = assets.tile_sets.create(
    movement_side_atlas,
    TileSetSliceUniform{
      .tile_size_px = {64, 64},
      .tile_orientation_x = TileOrientation::kNormal,
      .tile_orientation_y = TileOrientation::kNormal,
      .direction = TileSliceDirection::kRowWise,
      .start_offset = 12,
      .stop_after = 6,
    });
  SDE_ASSERT_TRUE(walking_right_frames.has_value());

  auto walking_left_frames = assets.tile_sets.create(
    movement_side_atlas,
    TileSetSliceUniform{
      .tile_size_px = {64, 64},
      .tile_orientation_x = TileOrientation::kFlipped,
      .tile_orientation_y = TileOrientation::kNormal,
      .direction = TileSliceDirection::kRowWise,
      .start_offset = 12,
      .stop_after = 6,
    });
  SDE_ASSERT_TRUE(walking_left_frames.has_value());

  auto walking_front_right_frames = assets.tile_sets.create(
    movement_front_side_atlas,
    TileSetSliceUniform{
      .tile_size_px = {64, 64},
      .tile_orientation_x = TileOrientation::kNormal,
      .tile_orientation_y = TileOrientation::kNormal,
      .direction = TileSliceDirection::kRowWise,
      .start_offset = 12,
      .stop_after = 6,
    });
  SDE_ASSERT_TRUE(walking_front_right_frames.has_value());

  auto walking_front_left_frames = assets.tile_sets.create(
    movement_front_side_atlas,
    TileSetSliceUniform{
      .tile_size_px = {64, 64},
      .tile_orientation_x = TileOrientation::kFlipped,
      .tile_orientation_y = TileOrientation::kNormal,
      .direction = TileSliceDirection::kRowWise,
      .start_offset = 12,
      .stop_after = 6,
    });
  SDE_ASSERT_TRUE(walking_front_left_frames.has_value());

  auto walking_back_right_frames = assets.tile_sets.create(
    movement_back_side_atlas,
    TileSetSliceUniform{
      .tile_size_px = {64, 64},
      .tile_orientation_x = TileOrientation::kNormal,
      .tile_orientation_y = TileOrientation::kNormal,
      .direction = TileSliceDirection::kRowWise,
      .start_offset = 12,
      .stop_after = 6,
    });
  SDE_ASSERT_TRUE(walking_back_right_frames.has_value());

  auto walking_back_left_frames = assets.tile_sets.create(
    movement_back_side_atlas,
    TileSetSliceUniform{
      .tile_size_px = {64, 64},
      .tile_orientation_x = TileOrientation::kFlipped,
      .tile_orientation_y = TileOrientation::kNormal,
      .direction = TileSliceDirection::kRowWise,
      .start_offset = 12,
      .stop_after = 6,
    });
  SDE_ASSERT_TRUE(walking_back_left_frames.has_value());



  // RUNNING ----------------------------------------

  auto running_front_frames = assets.tile_sets.create(
    movement_front_atlas,
    TileSetSliceUniform{
      .tile_size_px = {64, 64},
      .tile_orientation_x = TileOrientation::kNormal,
      .tile_orientation_y = TileOrientation::kNormal,
      .direction = TileSliceDirection::kRowWise,
      .start_offset = 6,
      .stop_after = 6,
    });
  SDE_ASSERT_TRUE(running_front_frames.has_value());

  auto running_back_frames = assets.tile_sets.create(
    movement_back_atlas,
    TileSetSliceUniform{
      .tile_size_px = {64, 64},
      .tile_orientation_x = TileOrientation::kNormal,
      .tile_orientation_y = TileOrientation::kNormal,
      .direction = TileSliceDirection::kRowWise,
      .start_offset = 6,
      .stop_after = 6,
    });
  SDE_ASSERT_TRUE(running_back_frames.has_value());

  auto running_right_frames = assets.tile_sets.create(
    movement_side_atlas,
    TileSetSliceUniform{
      .tile_size_px = {64, 64},
      .tile_orientation_x = TileOrientation::kNormal,
      .tile_orientation_y = TileOrientation::kNormal,
      .direction = TileSliceDirection::kRowWise,
      .start_offset = 6,
      .stop_after = 6,
    });
  SDE_ASSERT_TRUE(running_right_frames.has_value());

  auto running_left_frames = assets.tile_sets.create(
    movement_side_atlas,
    TileSetSliceUniform{
      .tile_size_px = {64, 64},
      .tile_orientation_x = TileOrientation::kFlipped,
      .tile_orientation_y = TileOrientation::kNormal,
      .direction = TileSliceDirection::kRowWise,
      .start_offset = 6,
      .stop_after = 6,
    });
  SDE_ASSERT_TRUE(running_left_frames.has_value());

  auto running_front_right_frames = assets.tile_sets.create(
    movement_front_side_atlas,
    TileSetSliceUniform{
      .tile_size_px = {64, 64},
      .tile_orientation_x = TileOrientation::kNormal,
      .tile_orientation_y = TileOrientation::kNormal,
      .direction = TileSliceDirection::kRowWise,
      .start_offset = 6,
      .stop_after = 6,
    });
  SDE_ASSERT_TRUE(running_front_right_frames.has_value());

  auto running_front_left_frames = assets.tile_sets.create(
    movement_front_side_atlas,
    TileSetSliceUniform{
      .tile_size_px = {64, 64},
      .tile_orientation_x = TileOrientation::kFlipped,
      .tile_orientation_y = TileOrientation::kNormal,
      .direction = TileSliceDirection::kRowWise,
      .start_offset = 6,
      .stop_after = 6,
    });
  SDE_ASSERT_TRUE(running_front_left_frames.has_value());

  auto running_back_right_frames = assets.tile_sets.create(
    movement_back_side_atlas,
    TileSetSliceUniform{
      .tile_size_px = {64, 64},
      .tile_orientation_x = TileOrientation::kNormal,
      .tile_orientation_y = TileOrientation::kNormal,
      .direction = TileSliceDirection::kRowWise,
      .start_offset = 6,
      .stop_after = 6,
    });
  SDE_ASSERT_TRUE(running_back_right_frames.has_value());

  auto running_back_left_frames = assets.tile_sets.create(
    movement_back_side_atlas,
    TileSetSliceUniform{
      .tile_size_px = {64, 64},
      .tile_orientation_x = TileOrientation::kFlipped,
      .tile_orientation_y = TileOrientation::kNormal,
      .direction = TileSliceDirection::kRowWise,
      .start_offset = 6,
      .stop_after = 6,
    });
  SDE_ASSERT_TRUE(running_back_left_frames.has_value());



  auto window_target_or_error = RenderTarget::create(app_or_error->window());
  SDE_ASSERT_TRUE(window_target_or_error.has_value());


  auto renderer_or_error = Renderer2D::create();
  SDE_ASSERT_TRUE(renderer_or_error.has_value());


  RenderResources sprite_rendering_resources;
  sprite_rendering_resources.shader = (*sprite_shader_or_error);
  sprite_rendering_resources.buffer_group = 0;

  RenderAttributes attributes;

  sde::Vec2f position{0, 0};
  sde::Vec2f direction{0, -1};
  sde::Vec2f direction_looking{0, -1};



  AnimatedSprite idle_front_animated_sprite{*idle_front_frames, 5.0F, AnimatedSprite::Mode::kLooped};
  AnimatedSprite idle_back_animated_sprite{*idle_back_frames, 5.0F, AnimatedSprite::Mode::kLooped};
  AnimatedSprite idle_right_animated_sprite{*idle_right_frames, 5.0F, AnimatedSprite::Mode::kLooped};
  AnimatedSprite idle_left_animated_sprite{*idle_left_frames, 5.0F, AnimatedSprite::Mode::kLooped};

  AnimatedSprite idle_front_right_animated_sprite{*idle_front_right_frames, 5.0F, AnimatedSprite::Mode::kLooped};
  AnimatedSprite idle_front_left_animated_sprite{*idle_front_left_frames, 5.0F, AnimatedSprite::Mode::kLooped};
  AnimatedSprite idle_back_right_animated_sprite{*idle_back_right_frames, 5.0F, AnimatedSprite::Mode::kLooped};
  AnimatedSprite idle_back_left_animated_sprite{*idle_back_left_frames, 5.0F, AnimatedSprite::Mode::kLooped};

  AnimatedSprite walking_front_animated_sprite{*walking_front_frames, 5.0F, AnimatedSprite::Mode::kLooped};
  AnimatedSprite walking_back_animated_sprite{*walking_back_frames, 5.0F, AnimatedSprite::Mode::kLooped};
  AnimatedSprite walking_left_animated_sprite{*walking_left_frames, 5.0F, AnimatedSprite::Mode::kLooped};
  AnimatedSprite walking_right_animated_sprite{*walking_right_frames, 5.0F, AnimatedSprite::Mode::kLooped};

  AnimatedSprite walking_front_right_animated_sprite{*walking_front_right_frames, 5.0F, AnimatedSprite::Mode::kLooped};
  AnimatedSprite walking_front_left_animated_sprite{*walking_front_left_frames, 5.0F, AnimatedSprite::Mode::kLooped};
  AnimatedSprite walking_back_right_animated_sprite{*walking_back_right_frames, 5.0F, AnimatedSprite::Mode::kLooped};
  AnimatedSprite walking_back_left_animated_sprite{*walking_back_left_frames, 5.0F, AnimatedSprite::Mode::kLooped};

  AnimatedSprite running_front_animated_sprite{*running_front_frames, 5.0F, AnimatedSprite::Mode::kLooped};
  AnimatedSprite running_back_animated_sprite{*running_back_frames, 5.0F, AnimatedSprite::Mode::kLooped};
  AnimatedSprite running_left_animated_sprite{*running_left_frames, 5.0F, AnimatedSprite::Mode::kLooped};
  AnimatedSprite running_right_animated_sprite{*running_right_frames, 5.0F, AnimatedSprite::Mode::kLooped};

  AnimatedSprite running_front_right_animated_sprite{*running_front_right_frames, 5.0F, AnimatedSprite::Mode::kLooped};
  AnimatedSprite running_front_left_animated_sprite{*running_front_left_frames, 5.0F, AnimatedSprite::Mode::kLooped};
  AnimatedSprite running_back_right_animated_sprite{*running_back_right_frames, 5.0F, AnimatedSprite::Mode::kLooped};
  AnimatedSprite running_back_left_animated_sprite{*running_back_left_frames, 5.0F, AnimatedSprite::Mode::kLooped};

  AnimatedSprite* prev_animation = &running_front_animated_sprite;
  AnimatedSprite* next_animation = &running_front_animated_sprite;

  app_or_error->spin([&](const auto& window)
  {
    const auto time = std::chrono::duration_cast<std::chrono::duration<float>>(window.time).count();
    const auto time_delta = std::chrono::duration_cast<std::chrono::duration<float>>(window.time_delta).count();


    // Handle screen zoom
    static constexpr float kScaleRate = 1.5;
    const float scroll_sensitivity = std::clamp(attributes.scaling, 1e-4F, 1e-2F);
    if (window.mouse_scroll.y() > 0)
    {
      attributes.scaling -= scroll_sensitivity * kScaleRate * time;
    }
    else if (window.mouse_scroll.y() < 0)
    {
      attributes.scaling += scroll_sensitivity * kScaleRate * time;
    }
    attributes.scaling = std::max(attributes.scaling, 1e-3F);
    attributes.time = time;
    attributes.time_delta = time_delta;



    direction.setZero();

    static constexpr float kSpeedWalking = 0.5;
    static constexpr float kSpeedRunning = 1.0;

    // Handle character speed
    const float next_speed =
      window.keys.isDown(KeyCode::kLShift) ? kSpeedRunning : kSpeedWalking;


    // Handle movement controls
    if (window.keys.isDown(KeyCode::kA))
    {
      direction.x() = -next_speed;
    }
    if (window.keys.isDown(KeyCode::kD))
    {
      direction.x() = +next_speed;
    }
    if (window.keys.isDown(KeyCode::kS))
    {
      direction.y() = -next_speed;
    }
    if (window.keys.isDown(KeyCode::kW))
    {
      direction.y() = +next_speed;
    }


    // Handle next animation
    if ((direction.x() > 0) and (direction.y() > 0))
    {
      next_animation = (next_speed == kSpeedWalking) ? &walking_back_right_animated_sprite : &running_back_right_animated_sprite;
    }
    else if ((direction.x() < 0) and (direction.y() > 0))
    {
      next_animation = (next_speed == kSpeedWalking) ? &walking_back_left_animated_sprite : &running_back_left_animated_sprite;
    }
    else if ((direction.x() > 0) and (direction.y() < 0))
    {
      next_animation = (next_speed == kSpeedWalking) ? &walking_front_right_animated_sprite : &running_front_right_animated_sprite;
    }
    else if ((direction.x() < 0) and (direction.y() < 0))
    {
      next_animation = (next_speed == kSpeedWalking) ? &walking_front_left_animated_sprite : &running_front_left_animated_sprite;
    }
    else if (direction.x() > 0)
    {
      next_animation = (next_speed == kSpeedWalking) ? &walking_right_animated_sprite : &running_right_animated_sprite;
    }
    else if (direction.x() < 0)
    {
      next_animation = (next_speed == kSpeedWalking) ? &walking_left_animated_sprite : &running_left_animated_sprite;
    }
    else if (direction.y() < 0)
    {
      next_animation = (next_speed == kSpeedWalking) ? &walking_front_animated_sprite : &running_front_animated_sprite;
    }
    else if (direction.y() > 0)
    {
      next_animation = (next_speed == kSpeedWalking) ? &walking_back_animated_sprite : &running_back_animated_sprite;
    }
    else if ((direction_looking.x() > 0) and (direction_looking.y() > 0))
    {
      next_animation = &idle_back_right_animated_sprite;
    }
    else if ((direction_looking.x() < 0) and (direction_looking.y() > 0))
    {
      next_animation = &idle_back_left_animated_sprite;
    }
    else if ((direction_looking.x() > 0) and (direction_looking.y() < 0))
    {
      next_animation = &idle_front_right_animated_sprite;
    }
    else if ((direction_looking.x() < 0) and (direction_looking.y() < 0))
    {
      next_animation = &idle_front_left_animated_sprite;
    }
    else if (direction_looking.x() > 0)
    {
      next_animation = &idle_right_animated_sprite;
    }
    else if (direction_looking.x() < 0)
    {
      next_animation = &idle_left_animated_sprite;
    }
    else if (direction_looking.y() < 0)
    {
      next_animation = &idle_front_animated_sprite;
    }
    else if (direction_looking.y() > 0)
    {
      next_animation = &idle_back_animated_sprite;
    }

    if ((direction.array() != 0.0F).any())
    {
      direction_looking = direction;
      next_animation->setRate(next_speed * 15.0F);
    }
    else
    {
      next_animation->setRate(kSpeedWalking * 15.0F);
    }

    next_animation->update(attributes.time);

    position += direction * time_delta;


    window_target_or_error->refresh(Black());
    if (auto render_pass_or_error = RenderPass::create(*window_target_or_error, *renderer_or_error, assets, attributes, sprite_rendering_resources); render_pass_or_error.has_value())
    {
      next_animation->draw(*render_pass_or_error, {position - sde::Vec2f{0.5, 0.5}, position + sde::Vec2f{0.5, 0.5}});
    }
    if (auto render_pass_or_error = RenderPass::create(*window_target_or_error, *renderer_or_error, assets, attributes, text_rendering_resources); render_pass_or_error.has_value())
    {
      type_setter.draw(*render_pass_or_error, "bob", position + sde::Vec2f{0.0, 0.35}, {0.075F});
      type_setter.draw(*render_pass_or_error, sde::format("pos: (%.3f, %.3f)", position.x(),  position.y()),  position + sde::Vec2f{0.0, -0.30}, {0.025F}, Yellow(0.8));
      type_setter.draw(*render_pass_or_error, sde::format("vel: (%.3f, %.3f)", direction.x(), direction.y()), position + sde::Vec2f{0.0, -0.35}, {0.025F}, Yellow(0.8));
    }
    prev_animation = next_animation;

    return AppDirective::kContinue;
  });

  SDE_LOG_INFO("done.");
  return 0;
}

// clang-format on
