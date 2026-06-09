#include <SFML/Graphics.hpp>

#include <cstdlib>
#include <optional>

int main()
{
    sf::RenderWindow window(
        sf::VideoMode({800, 600}),
        "Single Circle"
    );

    sf::Font font;
    if (!font.openFromFile("fonts/arial.ttf"))
    {
        return EXIT_FAILURE;
    }

    sf::CircleShape circle(50.0f);
    circle.setFillColor(sf::Color::Green);
    circle.setPosition({350.0f, 250.0f});

    sf::Text text(font, "Circle", 24);
    text.setFillColor(sf::Color::White);

    const sf::FloatRect circleBounds = circle.getLocalBounds();
    const sf::FloatRect textBounds = text.getLocalBounds();

    const sf::Vector2f circleCenter{
        circleBounds.position.x + circleBounds.size.x / 2.0f,
        circleBounds.position.y + circleBounds.size.y / 2.0f
    };

    const sf::Vector2f textCenter{
        textBounds.position.x + textBounds.size.x / 2.0f,
        textBounds.position.y + textBounds.size.y / 2.0f
    };

    text.setPosition(circle.getPosition() + circleCenter - textCenter);

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }

        window.clear(sf::Color::Black);
        window.draw(circle);
        window.draw(text);
        window.display();
    }

    return EXIT_SUCCESS;
}