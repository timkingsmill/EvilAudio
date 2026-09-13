target_sources(${PROJECT_NAME}
    PUBLIC

    PRIVATE
        controllers/BaseController.h
        controllers/BaseController.cpp
        controllers/ComponentController.h
        controllers/ComponentController.cpp
        controllers/WindowController.h
        controllers/WindowController.cpp

        bindings/PropertyBindings.h
        bindings/PropertyBindings.cpp

        datamodels/BaseDataModelObserver.h
        datamodels/BaseDataModel.h
        datamodels/BaseDataModel.cpp

        commands/BaseCommand.h
        commands/BaseCommand.cpp
        commands/DataModelCommand.h
        commands/DataModelCommand.cpp
        commands/NullCommand.h
        commands/NullCommand.cpp
        commands/CommandFactory.h

        viewmodels/BaseViewModel.h
        viewmodels/BaseViewModel.cpp
        viewmodels/ViewModelFactory.h

        views/BaseView.h
        views/BaseView.cpp
        views/BaseWindowView.h
        views/BaseWindowView.cpp
        views/BaseComponentView.h
        views/BaseComponentView.cpp
        views/BaseDialogWindowView.h
        views/BaseDialogWindowView.cpp
      
        validations/ValidationResult.h
        validations/ValidationResultFactories.h
)

