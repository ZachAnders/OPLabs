//
//  SessionData.m
//  Hermes
//
//  Created by Nicolas Charles Herbert Broeking on 2/12/15.
//  Copyright (c) 2015 NicolasBroeking. All rights reserved.
//

#import "SessionData.h"

@implementation SessionData
@synthesize sessionId;
@synthesize email, password, hostname;

+(SessionData*) getData
{
    static SessionData *sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[SessionData alloc] init];
    });
    return sharedInstance;
}
- (instancetype)init
{
    self = [super init];
    if (self) {
        sessionId = nil;
        hostname = nil;
        email = nil;
        password = nil;
    }
    return self;
}

//This method pulls the settings from the settings bundle
-(void)sync
{
    [[NSUserDefaults standardUserDefaults] synchronize];
    
    NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    
    hostname = [[standardUserDefaults objectForKey:@"hostname"] copy];
    email = [[standardUserDefaults objectForKey:@"email"] copy];
    password = [[standardUserDefaults objectForKey:@"password"] copy];
    
    
    NSLog(@"Data Settings:");
    NSLog(@"hostname: %@", hostname);
    NSLog(@"email: %@", email);
    NSLog(@"password: %@", password);
    
    //If they are null then we need to write the defaults and try again
    if( !hostname || !email || !password)
    {
        [self registerDefaultsFromSettingsBundle];
        [self sync];
    }
}

#pragma NSUserDefaults
//This method will only be called once. On the very first time that the app is launched
- (void)registerDefaultsFromSettingsBundle {
    NSLog(@"Writting Defaults");
    // this function writes default settings as settings
    NSDictionary *userDefaultsDefaults = [NSDictionary dictionaryWithObjectsAndKeys: @"https://128.138.202.143", @"hostname", @"", @"email", @"", @"password", nil];
    
    [[NSUserDefaults standardUserDefaults] registerDefaults:userDefaultsDefaults];
    
}
@end
