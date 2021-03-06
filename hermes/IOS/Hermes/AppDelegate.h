//
//  AppDelegate.h
//  Hermes
//
//  Created by Nicolas Charles Herbert Broeking on 1/17/15.
//  Copyright (c) 2015 NicolasBroeking. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <CoreData/CoreData.h>
#import "Communication.h"
#import "Tester.h"

@interface AppDelegate : UIResponder <UIApplicationDelegate>

@property (strong, nonatomic) UIWindow *window;

//We included objects to be able to store data for future versions currently not used at this time
@property (readonly, strong, nonatomic) NSManagedObjectContext *managedObjectContext;
@property (readonly, strong, nonatomic) NSManagedObjectModel *managedObjectModel;
@property (readonly, strong, nonatomic) NSPersistentStoreCoordinator *persistentStoreCoordinator;
@property (readonly, strong, atomic) Communication* comm;
@property (readonly, strong, atomic) Tester* tester;

- (void)saveContext;
- (NSURL *)applicationDocumentsDirectory;


@end

