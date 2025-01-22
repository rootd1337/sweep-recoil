#include <Windows.h>
#include <thread>
#include <chrono>
#include <iostream>

// Some additional includes.
#include <array>

// There's no real need for this to be a class. 
// Putting the functions in a namespace is more organized and simpler. 
namespace Mouse {
    void move(short x, short y) {
        mouse_event(MOUSEEVENTF_MOVE, x, y, 0, 0);
    }

    void click(short delay_ms) {
        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
    }
}

// Macros are cursed and should only ever be used in specific cases. Use constexpr instead. 
// Also, for clarity, put the colors in a namespace.
namespace Colors {
    // Declare them all as compile time string literals.
    constexpr const char* RESET_COLOR = "\033[0m";
    constexpr const char* GREEN = "\033[1;32m";
    constexpr const char* GOLD = "\033[1;33m";
    constexpr const char* LIGHT_GRAY = "\033[0;37m";
    constexpr const char* DARK_GRAY = "\033[1;30m";
}

// All of these variables should be constant, and since they're known at compile time,
// they should be constexpr. 
constexpr double sensitivity = 0.42; // The users sens should go here.
constexpr double default_sens = 2.52; // This should not be changed unless the recoil patterns themselves change. 
constexpr double scale_factor = default_sens / sensitivity; // By how much do the coordinates need to be scaled to work for the user's sensitivity? 

// Define the max and min recoil strength here, also helps avoid magic numbers. 
constexpr uint32_t strength_max = 10;
constexpr uint32_t strength_min = 1;

// There's no reason for the input and RCS to be running on separate threads. 
int main() {
    // Define most variables here. Remember that non-const globals are cursed.
    uint32_t recoil_strength = 10; // How strong should the recoil control be? (10 is no smooth, 1 is very smooth)
    double sleep = 99; // How many ms should we sleep between each shot?
    double delta_time = 0; // This will store how much time has elapsed between each cycle. 

    // Here, we can make use of the array class provided by the STL. 
    // Also, for simplicity, just store both X and Y coordinates in a pair. 
    std::array<std::pair<double, double>, 30> recoil_pattern = { { // Actual CS2 Ak-47 recoil pattern. 
        { 0, 0 },  { 0, 0 },  { 0.10497, -26.00426 },  { -2.49497, -29.9552 },  { -1.05429, -31.9007 },  { 12.3244, -33.1178 },  { 8.45939, -27.7364 },  { 13.1337, -19.8188 },  { -16.9038, -15.1133 },  { -42.0762, 5.02222 },  { -23.1968, -9.1423 },  { 13.4584, -7.82848 },  { -16.283, -1.73109 },  { -26.8526, 9.52371 },  { -2.44215, -5.35626 },  { 37.232, -8.02386 },  { 23.2809, -5.40851 },  { 14.3236, -5.75076 },  { 26.6208, 2.82855 },  { 34.9165, 8.60448 },  { -19.1055, -5.58987 },  { 5.08387, 0.156464 },  { -8.60053, -5.91907 },  { -9.06119, -2.09341 },  { 20.3148, 2.80682 },  { 6.87328, -5.38808 },  { -21.3522, -0.609879 },  { -33.0195, 2.71869 },  { -47.1704, 21.1572 },  { -14.4156, -0.197525 }
    } };

    // Now we need to scale the recoil pattern to the user's sens. 
    // Each coordinate needs to be multiplied by default_sens / user_sens
    for (auto& [x, y] : recoil_pattern) {
        x *= scale_factor; y *= scale_factor;
        y = -y; // Strange but the chinese github repo I stole the patterns from had the y flipped.
    }

    // Simple helper function to sleep for an accurate amount of time. Very CPU intensive though.  
    auto spin_lock = [](double ms) -> void {
        auto start = std::chrono::high_resolution_clock::now();

        // Check if enough time has elapsed since the start.
        // This might look a bit complex, but you simply get the timespan between the start and now, cast it to milliseconds and return once it's higher than the ms.
        while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() < ms) {}
        };

    // For the banner, we might as well make it a lambda to not have to pass all the misc variables.
    auto show_banner = [&recoil_strength]() -> void { // Make sure to capture recoil_strength by reference. 
        // All good. 
        system("cls");

        std::cout << Colors::GOLD << "============================" << Colors::RESET_COLOR << "\n";
        std::cout << Colors::GREEN << "     SWEEP RECOIL" << Colors::RESET_COLOR << "\n";
        std::cout << Colors::GOLD << "============================" << Colors::RESET_COLOR << "\n";
        std::cout << Colors::LIGHT_GRAY << "\nCurrent Recoil Strength: " << Colors::GREEN
            << recoil_strength << "/10" << Colors::RESET_COLOR << "\n";


        std::cout << Colors::LIGHT_GRAY << "Strength Level: [" << Colors::RESET_COLOR;
        // Let's reduce magic numbers. What if you wanted to make the recoil strength between 1-15? You'd have to change all this.
        for (int i = 0; i < strength_max; i++) {
            if (i < recoil_strength) {
                std::cout << Colors::GREEN << "#";
            }
            else {
                std::cout << Colors::LIGHT_GRAY << "-";
            }
        }
        std::cout << Colors::LIGHT_GRAY << "]" << Colors::RESET_COLOR << "\n";

        // Pressing 1-10 interferes with gameplay, no? Why not make it the left / right arrow keys?
        std::cout << "\n" << Colors::LIGHT_GRAY << "Press left / right arrow keys to change strenght." << Colors::RESET_COLOR << "\n";
        std::cout << Colors::LIGHT_GRAY << "Press 'q' to quit." << Colors::RESET_COLOR << "\n";
        };

    // Let's show the banner immediately.
    show_banner();

    // In your recoil thread you used for(;;), which is horrible at expressing intent.
    // Always use while(true) if you need an infinite loop.
    while (true) {
        auto start = std::chrono::high_resolution_clock::now(); // For deltaTime.
        // Control the recoil here.
        if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) && (GetAsyncKeyState(VK_RBUTTON) & 0x8000)) {
            // Use a range-based for loop here, since it is most fitting.
            for (const auto& [x, y] : recoil_pattern) {
                // Break if the user stops holding either mouse button.
                if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000) || !(GetAsyncKeyState(VK_RBUTTON) & 0x8000))
                    break;

                // Handle our recoil_strength. The more strong it is, the less smooth with a max strength of 10. 
                // Therefore we need the inverse of the recoil_strength.
                uint32_t strength_inverse = strength_max - recoil_strength + 1;

                // We will do the RCS a total of strength_inverse times, to achieve smoothing. 
                for (uint32_t i = 0; i < strength_inverse; i++) {
                    // Move the mouse.
                    // Divide by strength_inverse, as we'll be doing it repeatedly.
                    Mouse::move(x / strength_inverse, y / strength_inverse);
                    // We should actually be doing this once when the program started, but as the strength is variable, we cannot. This is less efficient

                    // Use our more accurate (but also more CPU-intensive) sleep function here.
                    // We do not want to delay for too much. So remove the delta_time from the sleep.                
                    spin_lock((sleep - delta_time) / strength_inverse); // Also divide by strength_inverse here.  
                }

                delta_time = 0; // We have already 'gotten rid' of the delta_time. Therefore set it to zero, as we don't wanna do this for each bullet. 
            }
        }

        // Handle input here.

        // I noticed you used _kbhit() and _getch() in your code. 
        // No need to use those, they're very outdated and you're already using GetAsyncKeyState, might as well utilize that. 

        if (GetAsyncKeyState('Q')) // Quit.
            return 0;

        // Nothing special, just inc / decrement the recoil_strength based on the input.
        if (GetAsyncKeyState(VK_RIGHT) && recoil_strength < strength_max) {
            recoil_strength++; show_banner();
            // Sleep here, simplest way to prevent the user from accidentally incrementing too much. (Very hacky though.) 
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        else if (GetAsyncKeyState(VK_LEFT) && recoil_strength > strength_min) {
            recoil_strength--; show_banner();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        // Same calculation as in the spin_lock.
        delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
    }

    return 0;
}