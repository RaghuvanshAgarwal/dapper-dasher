#include <iostream>
#include "raylib.h"


class scarfy
{
    const int jump_force_{900}; // pixels/frame
    const int gravity_{2'000}; // (pixels/frame)/frame
    const int player_width_{128};
    const int player_height_{128};
    const float update_frame_time_{1 / 10.0f};
    bool is_in_air_;
    int index_ = 0;
    int velocity_{0};
    float player_ground_;
    float player_x_;
    float timer_ = 0;
    Texture2D* texture_;
    Rectangle* rect_;
    Vector2* pos_;

public:
    scarfy(const int window_width, const int window_height, Texture2D* texture)
    {
        player_ground_ = {static_cast<float>(window_height - player_height_)};
        player_x_ = {static_cast<float>(window_width - player_width_) * 0.25f};
        texture_ = texture;
        is_in_air_ = false;
        rect_ = new Rectangle {player_x_, 0, static_cast<float>(player_width_), static_cast<float>(player_height_)};
        pos_ = new Vector2{player_x_, player_ground_};
    }

    ~scarfy()
    {
        delete rect_;
        delete pos_;
    }

    void on_update()
    {
        //Perform Ground Check
        if (pos_->y >= player_ground_)
        {
            pos_->y = {player_ground_};
            velocity_ = 0;
            is_in_air_ = false;
        }
        else
        {
            velocity_ += static_cast<int>(static_cast<float>(gravity_) * GetFrameTime());
            is_in_air_ = true;
        }

        //Check for Jump
        if (IsKeyPressed(KEY_SPACE) && !is_in_air_)
        {
            velocity_ -= jump_force_;
        }

        // Add Velocity to Position
        pos_->y += static_cast<float>(velocity_) * GetFrameTime();
        timer_ += GetFrameTime();
        if (timer_ > update_frame_time_ && !is_in_air_)
        {
            timer_ = 0;
            index_ = (index_ + 1) % 6;
            rect_->x = static_cast<float>(player_width_ * index_);
        }
    }

    void on_draw() const
    {
        DrawTextureRec(*texture_, *rect_, *pos_, WHITE);
    }

    Rectangle get_rect() const
    {
        return Rectangle{pos_->x,pos_->y, rect_->width,rect_->height};
    }
};

class background
{
    float speed_;

    Texture2D* texture_;
    Rectangle* source_;
    Rectangle* dest_;
public:
    background(const float speed, const int window_width, const int window_height, Texture2D* texture)
    {
        speed_ = speed;
        texture_ = texture;
        source_ = new Rectangle{0, 0, static_cast<float>(texture_->width), static_cast<float>(texture_->height)};
        dest_ = new Rectangle {0, 0, static_cast<float>(window_width), static_cast<float>(window_height)};
    }

    ~background()
    {
        delete source_;
        delete dest_;
    }


    void on_update() const
    {
        source_->x += speed_ * GetFrameTime();
    }

    void on_draw() const
    {
        DrawTexturePro(*texture_,*source_,*dest_,{0, 0},0,WHITE);
    }

    void set_speed(float new_speed)
    {
        speed_ = new_speed;
    }
};

class nebula
{
    const int total_frame_ {60};
    const int row_ {8};
    const int columns {8};
    int curr_frame_ {0};
    float curr_time_ {0.0f};
    float time_to_update_ {0.1f};
    
    Texture2D* texture_;
    Rectangle rect_;
    Vector2 position_;
    float speed_;
    

public:

    nebula()
    {
        texture_ = nullptr;
        rect_ = {0,0,0,0};
        position_ = {0,0};
        speed_ = 0;
    }
    
    nebula(const Texture2D* texture, Vector2 position, float speed)
    {
        texture_ = const_cast<Texture2D*>(texture);
        position_ = position;
        rect_ = {0,0,100,100};
        speed_ = speed;
    }

    nebula(const nebula& other)
        : curr_frame_(other.curr_frame_), curr_time_(other.curr_time_), texture_(other.texture_),
          rect_(other.rect_), position_(other.position_), speed_(other.speed_)
    {
    }

    // Add copy assignment operator
    nebula& operator=(const nebula& other)
    {
        if (this != &other)
        {
            texture_ = other.texture_;
            rect_ = other.rect_;
            position_ = other.position_;
            curr_frame_ = other.curr_frame_;
            curr_time_ = other.curr_time_;
            speed_ = other.speed_;
        }
        return *this;
    }

    void update_sprite_frame()
    {
        curr_time_ += GetFrameTime();
        if(curr_time_ > time_to_update_)
        {
            curr_time_ = 0;
            curr_frame_ = (curr_frame_ + 1) % total_frame_;
            rect_.x = static_cast<float>(curr_frame_ % row_) * rect_.width;
            rect_.y = static_cast<int>(curr_frame_ / columns) * rect_.height;
        }
    }

    void on_update()
    {
        update_sprite_frame();
        position_.x -= speed_ * GetFrameTime();
    }

    void on_draw() const
    {
        if(texture_ == nullptr) return;
        DrawTextureRec(*texture_, rect_, position_, WHITE);
    }

    const Vector2* get_position() const
    {
        return &position_;
    }

    void set_position(const Vector2 new_position)
    {
        position_ = new_position;
    }

    Rectangle get_rect() const
    {
        return Rectangle{position_.x,position_.y, rect_.width,rect_.height};
    }
};

int main()
{
    constexpr int window_width{512};
    constexpr int window_height{380};
    constexpr int number_of_nebulae = 10;

    InitWindow(window_width, window_height, "Dapper Dasher");
    SetTargetFPS(60);

    Texture2D scarfy_texture = LoadTexture("./textures/scarfy.png");
    Texture2D far_building_texture = LoadTexture("./textures/far-buildings.png");
    Texture2D back_building_texture = LoadTexture("./textures/back-buildings.png");
    Texture2D foreground_texture = LoadTexture("./textures/foreground.png");
    const Texture2D nebula_spritesheet = LoadTexture("./textures/12_nebula_spritesheet.png");

    scarfy player(window_width, window_height, &scarfy_texture);
    const background far_building(50,window_width,window_height, &far_building_texture);
    const background back_building(100,window_width,window_height, &back_building_texture);
    const background foreground(200,window_width,window_height, &foreground_texture);
    nebula nebula_arr[number_of_nebulae];

    int game_state {0}; // 0 -> In progress, 1 -> Game Finished [Success], 2 -> Game Over [Fail]

    for (size_t i = 0 ; i  < number_of_nebulae ; i++)
    {
        constexpr float nebulae_speed = 400;
        const float x = static_cast<float>(window_width + i * 450); // Space them out by 300 pixels
        const float y = static_cast<float>(window_height - 150 + 10 * (i % 3)); // Vary y position slightly
        const Vector2 nebula_pos = {x, y};
        
        nebula_arr[i] = nebula(&nebula_spritesheet, nebula_pos, nebulae_speed);
    }

    const nebula* last_nebula = &nebula_arr[number_of_nebulae - 1];

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        far_building.on_update();
        back_building.on_update();
        foreground.on_update();
        far_building.on_draw();
        back_building.on_draw();
        foreground.on_draw();
        switch (game_state)
        {
        case 0:
            player.on_update();
            for (auto& i : nebula_arr)
            {
                i.on_update();
            }
            
            player.on_draw();
            for (const auto& i : nebula_arr)
            {
                i.on_draw();
            }

            // checking for winning state
            if(last_nebula->get_position()->x < -500)
            {
                game_state = 1;
            }

            for(const auto& i: nebula_arr)
            {
                const Vector2* position = i.get_position();
                const Rectangle rect = i.get_rect();
                constexpr int radius {20};
                Vector2 center = {position->x + (rect.width/2), position->y + (rect.height/2)};
                if(CheckCollisionCircleRec(center,radius,player.get_rect()))
                {
                    game_state = 2;
                }
            }

            
            break;
        case 1:

            {
                const auto escape = "You were able to escape";
                const int width = MeasureText(escape, 20);
                DrawText(escape, (window_width - width) /2, window_height/2,20,WHITE);
            }
            break;
        case 2:
            {
                const auto fail = "You were not able to escape";
                const int width = MeasureText(fail, 20);
                DrawText(fail, (window_width - width) /2, window_height/2,20,WHITE);
            }
            break;
        default:
                std::cout << "Something went wrong, game_state should never be any value apart from 0,1,2" << "\n";
                break;
        }

        if(game_state == 1 || game_state == 2)
        {
            if(IsKeyPressed(KEY_ENTER))
            {
                for (size_t i = 0 ; i  < number_of_nebulae ; i++)
                {
                    const float x = static_cast<float>(window_width + i * 450); // Space them out by 300 pixels
                    const float y = static_cast<float>(window_height - 150 + 10 * (i % 3)); // Vary y position slightly
                    const Vector2 nebula_pos = {x, y};
                    nebula_arr[i].set_position(nebula_pos);
                }
                game_state = 0;
            }
            
        }
        
        EndDrawing();
    }
    UnloadTexture(scarfy_texture);
    UnloadTexture(far_building_texture);
    UnloadTexture(back_building_texture);
    UnloadTexture(foreground_texture);
    CloseWindow();
}
