#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <iostream>

#include "board.h"

int main()
{
  // Create the main window
  float window_width = 800.f;
  float window_height = 1000.f;

  float panel_height = 200.f;

  sf::RenderWindow app(sf::VideoMode(window_width, window_height), "2048 Developed by Richard's Dad");
  // Set window position on the screen
  app.setPosition(sf::Vector2i(800, 0));
  tgui::Gui gui(app);

  srand(time(NULL));

  sf::Font font;
  if (!font.loadFromFile("DroidSansMono.ttf")) {
      std::cout << "Font load failed: "
          << "DroidSansMono.ttf" << std::endl;
  }

  Board b(&app, &font, 0.f, panel_height, window_width);
  //b.SetNumber(1024, 2, 2);
  //b.SetNumber(256, 2, 3);
  //b.SetNumber(2, 0, 0);
  //b.SetNumber(4, 0, 1);
  //b.SetNumber(2048, 3, 3);
  b.PrintBoard(b.GetCurrentBoard());

  // New game button
  auto new_game_button = tgui::Button::create();
  new_game_button->getRenderer()->setBackgroundColor(sf::Color::White);
  new_game_button->getRenderer()->setBackgroundColorHover(sf::Color::Red);
  new_game_button->setPosition(700, 100);
  new_game_button->setText("New Game");
  new_game_button->setTextSize(18);
  new_game_button->setSize(100, 40);
  new_game_button->onClick([&] {b.ResetBoard(b.GetCurrentBoard()); });
  gui.add(new_game_button);

  // Load button
  auto load_button = tgui::Button::create();
  load_button->getRenderer()->setBackgroundColor(sf::Color::White);
  load_button->getRenderer()->setBackgroundColorHover(sf::Color::Red);
  load_button->setPosition(600, 100);
  load_button->setText("Load");
  load_button->setTextSize(18);
  load_button->setSize(100, 40);
  load_button->onClick([&] {b.LoadFromFile(b.GetCurrentBoard()); });
  gui.add(load_button);

  // Save button
  auto save_button = tgui::Button::create();
  save_button->getRenderer()->setBackgroundColor(sf::Color::White);
  save_button->getRenderer()->setBackgroundColorHover(sf::Color::Red);
  save_button->setPosition(500, 100);
  save_button->setText("Save");
  save_button->setTextSize(18);
  save_button->setSize(100, 40);
  save_button->onClick([&] {b.SaveToFile(); });
  gui.add(save_button);

  // Score label
  auto score_label = tgui::Label::create();
  score_label->getRenderer()->setBackgroundColor(sf::Color::White);
  score_label->setPosition(500, 20);
  score_label->setText("Score");
  score_label->setTextSize(18);
  score_label->setSize(145, 40);
  gui.add(score_label);

  // Best score label
  auto best_score_label = tgui::Label::create();
  best_score_label->getRenderer()->setBackgroundColor(sf::Color::White);
  best_score_label->setPosition(650, 20);
  best_score_label->setText("Best");
  best_score_label->setTextSize(18);
  best_score_label->setSize(145, 40);
  gui.add(best_score_label);

  // Human-bot input checkbox
  auto input_mode_checkbox = tgui::CheckBox::create();
  input_mode_checkbox->setPosition(100, 50);
  input_mode_checkbox->setText("Human input");
  input_mode_checkbox->setSize(30, 30);
  input_mode_checkbox->setChecked(true);
  input_mode_checkbox->onClick([&] {b.ToggleInputMode(); });
  gui.add(input_mode_checkbox);

  sf::Clock clock;
  float dt = 0;

  while (app.isOpen()) {
      dt += clock.restart().asSeconds();

      sf::Event event;
      while (app.pollEvent(event)) {
          if (event.type == sf::Event::Closed) {
              app.close();
          }

          gui.handleEvent(event);

          if (event.type == sf::Event::KeyPressed) {
              if (event.key.code == sf::Keyboard::Up && b.IsHumanInput()) {
                  b.HumanMove(SWIPE_DIRECTION::UP, b.GetCurrentBoard());
                  b.PrintBoard(b.GetCurrentBoard());
              }
              if (event.key.code == sf::Keyboard::Down && b.IsHumanInput()) {
                  b.HumanMove(SWIPE_DIRECTION::DOWN, b.GetCurrentBoard());
                  b.PrintBoard(b.GetCurrentBoard());
              }
              if (event.key.code == sf::Keyboard::Right && b.IsHumanInput()) {
                  b.HumanMove(SWIPE_DIRECTION::RIGHT, b.GetCurrentBoard());
                  b.PrintBoard(b.GetCurrentBoard());
              }
              if (event.key.code == sf::Keyboard::Left && b.IsHumanInput()) {
                  b.HumanMove(SWIPE_DIRECTION::LEFT, b.GetCurrentBoard());
                  b.PrintBoard(b.GetCurrentBoard());
              }
          }
      }

      app.clear();
    
      if (!b.IsHumanInput() && !b.GetCurrentBoard().is_dead) {
          if (dt >= b.GetBotMoveInterval()) {
              std::cout << "dt: " << dt << " seconds." << std::endl;
              dt = 0.f;
              b.BotMove(b.GetCurrentBoard());
              b.PrintBoard(b.GetCurrentBoard());
          }
      }
      b.Draw();
      score_label->setText("Score: " + std::to_string(b.GetScore(b.GetCurrentBoard())));
      best_score_label->setText("Best: " + std::to_string(b.GetBestScore()));
      gui.draw();
      app.display();
  }

  return 0;
}