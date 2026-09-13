#pragma once

#include <memory>

#include <juce_core.h>

#include <mvc/controllers/WindowController.h>

#include "MainAppDataModel.h"

/**
 * @defgroup evil_architecture Evil DAW Architecture and Design Principles
 * @brief Core architectural patterns and design principles for Evil Audio implementation.
 * 
 * @details
 * This module contains a best practices MVC implementation for EvilDAW.
 * It does NOT reflect the actual implementation of the application.
 * The real implementation may deviate from these patterns.
 * The main purpose of this module is to serve as a reference for architectural
 * best practices and design principles. It provides a clean, idealized example of 
 * how to structure the main application controller in an MVC-based application.
 *
 * @section evil_architecture_mvc Model-View-Controller (MVC) Pattern.
 * The MVC pattern is a software architectural pattern that separates an 
 * application into three main components: the Model, the View, and 
 * the Controller. This separation allows for modularity, maintainability, 
 * and scalability in application design.
 * 
 * @subsection evil_architecture_mvc_overview MVC Overview
 * - **Model**: Represents the application's data and business logic. It is responsible for managing the state of the application and notifying observers of any changes.
 * - **View**: Represents the user interface and presentation layer. It displays the data from the Model and sends user interactions to the Controller.
 * - **Controller**: Acts as an intermediary between the Model and the View. It processes
 * @plantumlfile docs/puml/diagrams/best_practices/mvc_best_practice.puml "Model-View-Controller (MVC) Pattern"
 
 * @subsection evil_architecture_mvc_command_undo MVC with Command and Undo
    * In a more complex application, the Controller can also implement the 
    * Command pattern to handle user actions and support undo functionality.
 * @plantumlfile docs/puml/diagrams/best_practices/main_app_controller_class_example.puml "MainAppController MVC Class Relationships"
 * 
 * @subsection evil_architecture_command_undo_sequence Command and Undo Sequence
 * This sequence diagram illustrates how the Controller processes a command and supports undo functionality.
 * @plantumlfile docs/puml/diagrams/best_practices/main_app_controller_sequence_example.puml "MainAppController Command and Undo Sequence"
 * 
 * @section evil_architecture_command_pattern Command Pattern
 * The Command pattern is a behavioral design pattern that encapsulates a request
 * as an object, thereby allowing for parameterization of clients with 
 * queues, requests, and operations.
 * 
 * 
 * XXplantumlfile evil_patterns_lib/mvc_multi_views_class_diagram.puml
 * XXplantumlfile evil_patterns_lib/mvc_multi_views_sequence_diagram.puml
 *
 * @subsection evil_architecture_embedded_puml_example Embedded PlantUML Class Diagram Example
 * See the standalone PlantUML example referenced above:
 * @ref docs/puml/diagrams/best_practices/main_app_controller_class_example.puml
 * This keeps the generated documentation concise while still providing a reusable
 * source diagram for the MVC class relationships.
 *
 * @subsection evil_architecture_embedded_sequence_example Embedded PlantUML Sequence Diagram Example
 * See the standalone PlantUML example referenced above:
 * @ref docs/puml/diagrams/best_practices/main_app_controller_sequence_example.puml
 * This keeps the generated documentation concise while still providing a reusable
 * source diagram for the command execution and undo flow.
 *
 * @section evil_mvc_cpp_example Example MVC C++ Code Example
 * The following C++ code demonstrates a simple implementation of the MVC pattern
 * with command and undo functionality.
 * @code{.cpp}
 * #include <iostream>
 * #include <vector>
 * #include <stack>
 * #include <memory>
 * #include <algorithm>
 *
 * // --- 1. Observer Interface ---
 * class Observer {
 * public:
 *     virtual void update(int value) = 0;
 * };
 *
 * // --- 2. Model (Subject) ---
 * class DataModel {
 *     int value = 0;
 *     std::vector<Observer*> observers;
 * public:
 *     void addObserver(Observer* o) { observers.push_back(o); }
 *     void notify() { for(auto o : observers) o->update(value); }
 *
 *     void setValue(int v) { value = v; notify(); }
 *     int getValue() const { return value; }
 * };
 *
 * // --- 3. Command with Undo ---
 * class Command {
 * public:
 *     virtual ~Command() = default;
 *     virtual void execute() = 0;
 *     virtual void undo() = 0;
 * };
 *
 * class ChangeValueCommand : public Command {
 *     DataModel& model;
 *     int prevValue, newValue;
 * public:
 *     ChangeValueCommand(DataModel& m, int val) : model(m), newValue(val) {
 *         prevValue = m.getValue();
 *     }
 *     void execute() override { model.setValue(newValue); }
 *     void undo() override { model.setValue(prevValue); }
 * };
 *
 * // --- 4. Controller (Invoker with History) ---
 * class Controller {
 *     std::stack<std::unique_ptr<Command>> undoStack;
 * public:
 *     void execute(std::unique_ptr<Command> cmd) {
 *         cmd->execute();
 *         undoStack.push(std::move(cmd));
 *     }
 *     void undo() {
 *         if (!undoStack.empty()) {
 *             undoStack.top()->undo();
 *             undoStack.pop();
 *         }
 *     }
 * };
 *
 * // --- 5. View (Observer) ---
 * class ConsoleView : public Observer {
 * public:
 *     void update(int value) override {
 *         std::cout << "[View] Model state updated to: " << value << "\n";
 *     }
 * };
 *
 * int main() {
 *     DataModel model;
 *     ConsoleView view;
 *     Controller controller;
 *
 *     model.addObserver(&view); // View registers to Model
 *
 *     // User interaction
 *     controller.execute(std::make_unique<ChangeValueCommand>(model, 10));
 *     controller.execute(std::make_unique<ChangeValueCommand>(model, 25));
 *
 *     std::cout << "--- Undoing ---\n";
 *     controller.undo();
 *
 *     return 0;
 * }
 * @endcode

 */
namespace evil::daw
{
    class BaseCommand;
    class BaseDataModel;

    class MainAppController final : public mvc::WindowController
    {
    public:
        MainAppController();
        ~MainAppController() override;

        using BaseController::setDataModel;
        void setDataModel(std::shared_ptr<mvc::BaseDataModel> model) override;

    protected:
        bool onCanExecuteCommand(const mvc::BaseCommand& command) override;
        mvc::ValidationResult onExecuteCommand(const mvc::BaseCommand& command) override;

    private:
        std::weak_ptr<MainAppDataModel> _typedModelWeak;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainAppController);
    };
}